#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <string.h>
/* CUSTOM LIBRARY */
#include "int96.h"


#define MAX_EAVESDROPPED 500
int DP_ROUND_COUNT = 0;
int USE_ROTATE = 0;
time_t t;

unsigned int hammingDistance(int96 op1,int96 op2);
//------------------------------------------------
// DAVID PRASAD'S PROTOCOL (SIMULATION FUNCTIONS)
//-------------------------------------------------

/* Ressembles the information that an RFID entity holds in the David Prasad protocol (either a tag or a reader)*/
typedef struct rfid_entity{
    int96 pid;
    int96 pid2;
    int96 k1;
    int96 k2;
    int96 id;
    int96 n1;
    int96 n2;
    int96 D;
} tag,reader;

void printProtocolState(tag * t, reader * r){

    printf("|---------------------------------TAG----------------------------------------|\n");
    char placeholder [24]; //Array used to hold each field before printing
    toHexString(t->id,placeholder);
    printf("|------id=0x%s-----------------------------------------|\n",placeholder);
    toHexString(t->pid,placeholder);
    printf("|------pid=0x%s----------------------------------------|\n",placeholder);
    toHexString(t->pid2,placeholder);
    printf("|------pid2=0x%s---------------------------------------|\n",placeholder);
    toHexString(t->k1,placeholder);
    printf("|------k1=0x%s-----------------------------------------|\n",placeholder);
    toHexString(t->k2,placeholder);
    printf("|------k2=0x%s-----------------------------------------|\n",placeholder);
    toHexString(t->n1,placeholder);
    printf("|------n1=0x%s-----------------------------------------|\n",placeholder);
    toHexString(t->n2,placeholder);
    printf("|------n2=0x%s-----------------------------------------|\n\n",placeholder);

    printf("|--------------------------------READER--------------------------------------|\n");
    toHexString(r->id,placeholder);
    printf("|------id=0x%s-----------------------------------------|\n",placeholder);
    toHexString(r->pid,placeholder);
    printf("|------pid=0x%s----------------------------------------|\n",placeholder);
    toHexString(r->pid2,placeholder);
    printf("|------pid2=0x%s---------------------------------------|\n",placeholder);
    toHexString(r->k1,placeholder);
    printf("|------k1=0x%s-----------------------------------------|\n",placeholder);
    toHexString(r->k2,placeholder);
    printf("|------k2=0x%s-----------------------------------------|\n",placeholder);
    toHexString(r->n1,placeholder);
    printf("|------n1=0x%s-----------------------------------------|\n",placeholder);
    toHexString(r->n2,placeholder);
    printf("|------n2=0x%s-----------------------------------------|\n",placeholder);



}

//Initialises the protocol for each end of the communication (tag & reader)
bool Init_David_Prasad( int96 ID , int96 PID , int96 PID2, int96 k1, int96 k2, tag * tag_t, reader * reader_t){

    setRand96_Seed(time(0));
    //Tag data at t0
    tag_t->id = ID;
    tag_t->pid = PID;
    tag_t->pid2 = PID2;
    tag_t->k1 = k1; 
    tag_t->k2 = k2;

    //Initialised here but not really known at startup
    tag_t->n1 = rand96();
    tag_t->n2 = rand96();

    //Reader data at t0
    reader_t->pid = PID;
    reader_t->pid2 = PID2;
    reader_t->k1 = k1;
    reader_t->k2 = k2;

    //Initialised here but not really known at startup
    reader_t->id = rand96();
    reader_t->n1 = rand96();
    reader_t->n2 = rand96();
}



//------------------------------------------------------------------------------
// MESSAGE GENERATION, EXCHANGE and EAVESDROPPING (implemented as return value)
//------------------------------------------------------------------------------
/*[OK]*/int96 sendPseudonym2(tag * t){
    return (t->pid2);
}
/*[OK]*/int96 sendPseudonym(tag * t){
    return (t->pid);
}
/*[OK]*/int96 sendA(reader r , tag * t){
    int96 out = xor(and3(r.pid2,r.k1,r.k2),r.n1); //Message itself, the one that is kind of encrypted
    //Receiver (tag) work simulated
    t->n1 = xor(out, and3(t->pid2, t->k1, t->k2)) ; //Extract n1 from A
    //Check correctness of protocol
    //testAssertionPrint("sendA",int96Equals(r.n1,t->n1), NULL); //check whether receiver was able to obtain n1 correctly 
    return out; //Return to simulate eavesdropping
}
/*[OK]*/int96 sendB(reader r, tag * t){
    int96 out =  xor(and3(not(r.pid2),r.k1,r.k2), r.n2);
    //Receiver (tag) work simulated
    t->n2 = xor(and3(not(t->pid2),t->k1,t->k2),out); //Extract n2 from B
     //Check correctness of protocol
    //testAssertionPrint("sendB",int96Equals(r.n2,t->n2), NULL); //check whether receiver was able to obtain n1 correctly 
    return out; //Return to simulate eavesdropping
}
/*[OK]*/int96 sendD(reader * r, tag * t){
    int96 out =  xor(and(r->k1,r->n2),and(r->k2,r->n1));
    if( USE_ROTATE > 0) rotateBits(&out,(r->n1.lsB));
    r->D = out;
    int96 localD = xor(and(t->k1,t->n2),and(t->k2,t->n1)); // tag computes D locally from previously exchanged parameters
    if(USE_ROTATE > 0) rotateBits(&localD,(t->n1.lsB));
    t->D = localD;
    //Confirmation that the reader is trustworthy because it knows both the keys and randon numbers
    //testAssertionPrint("sendD; get D'",int96Equals(out,localD),NULL);
    return out;
}
/*[OK]*/int96 sendE(tag t, reader * r){
    int96 out;

    if(USE_ROTATE == 2){ /*BUGGY*/
        out = xor3(t.k1,t.n1,t.id);
        rotateBits(&out, -(t.n2.lsB)); //left rotate
        int96 outPart = and(t.k2,t.n2);
        rotateBits(&outPart,t.n1.lsB); //right rotate
        out = xor(out,outPart);

        //Compute rotate(k1 & n2,n1)
        int96 inPartB = and(r->k2,r->n2);
        rotateBits(&inPartB, r->n1.lsB);
        //if(int96Equals(inPartB,outPart)){printf("PrintPart[OK]\n");}

        //Compute rotate(k1 xor n1, n2)
        int96 localk1N1 = xor(r->k1,r->n1);
        rotateBits(&localk1N1,-(r->n2.lsB));

        int96 idrotated = xor3(out,localk1N1,inPartB) ;
        //Obtain id
        rotateBits(&idrotated,(r->n2.lsB));
        r->id = idrotated;
        if(!int96Equals(t.id,r->id)) {
            char strTid[24];
            char strRid[24];
            toHexString(t.id,strTid);
            toHexString(r->id,strRid);
            printf("IDs not equal; hd(r->id,t->id) = %d \n\tt.id = 0x%s\n\tr->id= 0x%s\n", hammingDistance(t.id,r->id),strTid,strRid);

        }

    }else{
        out = xor( xor3(t.k1,t.n1,t.id), and(t.k2,t.n2) ) ;
        //Reader extracts ID from message
        r->id = xor3(r->k1, r->n1, xor(out,and(r->k2,r->n2)));
    }
    
    //testAssertionPrint("sendE; exchange ID", int96Equals(t.id,r->id), NULL);
    return out;
}
/*-Update step, preparing for next session-*/
/*[OK]*/bool pidsUpdate(tag *t, reader * r) {
    //Update pseudonyms at tag
    t->pid = t->pid2;
    t->pid2 = xor3(t->pid2,t->n1,t->n2);
    //Update pseudonyms at reader
    r->pid = r->pid2;
    r->pid2 = xor3(r->pid2,r->n1,r->n2);

    //Update randomNumbers
    int96 prevn1 = r->n1;
    r->n1 = rand96();
    //printf("n1_0:"); printHex(prevn1); printf("n1_new:"); printHex(r->n1);
    int96 prevn2 = r->n2;
    r->n2 = rand96();
    //printf("n2_0:"); printHex(prevn2); printf("n2_new:"); printHex(r->n2);
    
    return testAssertionPrint("Update PIDS - Round",int96Equals(t->pid,r->pid) && int96Equals(t->pid2,r->pid2), &DP_ROUND_COUNT);
}
/*[OK]*/int96 sendF(tag * t ,reader * r){
    int96 out = xor(and(t->k1,t->n1), and(t->k2,t->n2));
    //printHex(out);
    //Notification that Pseudonyms should be updated
    int96 fPrime = xor( and(r->k1,r->n1), and(r->k2,r->n2) );
    //printHex(fPrime);
    //testAssertionPrint("sendF; F' = F", int96Equals(fPrime,out), NULL);
    if(!int96Equals(out,fPrime)){printf("[Error] in Exchange 'F': F' is different at R");}
    if(!pidsUpdate(t,r)){printf("An error occurred on update PIDS\n");}
    return out;
}

/*Represents a set of eavesdropped messages for a single David-Prasad authentication session; comprises all the messages interchanged among tag and reader */
typedef struct {
    int96 pid2;
    int96 pid;
    int96 A;
    int96 B;
    int96 D;
    int96 E;
    int96 F;
} leakedSession;

typedef enum approximationParamEnum {ID,KEY1,KEY2} approximationParam ;


/* Represents a set of approximations for a secret key that are later used to derive a final approximation that aims to be equal to the actual key */
typedef struct goodApproximations{
    int96 a1;
    int96 a2;
    int96 a3;
    int96 a4;
    int96 a5;
    int96 a6;
    int96 a7;
    int96 a8;
    approximationParam param;
} GA_K1,GA_K2,GA_ID;


void printApproximationSet(struct goodApproximations ga, int roundID){
    printf("[Approximation] of %s (Round %d) \n",ga.param == 0  ? "ID":ga.param == 1 ? "KEY1": "KEY2" ,roundID);
    char aux [24]; //Holds ,defthe hex value momentarily
    char def [26]; // Holds the approximation definition momentarily
    toHexString(ga.a1,aux);
    switch(ga.param){
        case ID:
            sprintf(def,"NOT(E XOR F)");
            break;
        case KEY1:
            sprintf(def,"D");
            break;
        case KEY2:
            sprintf(def,"D");
            break;
    }
    printf("a1 (%s):0x%s\n",def,aux);
    
    toHexString(ga.a2,aux);
    switch(ga.param){
        case ID:
            sprintf(def,"A XOR B XOR E");
            break;
        case KEY1:
            sprintf(def,"F");
            break;
        case KEY2:
            sprintf(def,"F");
            break;
    }
    printf("a2 (%s):0x%s\n",def,aux);
    
    toHexString(ga.a3,aux);
    switch(ga.param){
        case ID:
            sprintf(def,"A XOR D XOR E");
            break;
        case KEY1:
            sprintf(def,"A XOR D");
            break;
        case KEY2:
            sprintf(def,"NOT(A XOR D)");
            break;
    }
    printf("a3 (%s):0x%s\n",def,aux);
    
    toHexString(ga.a4,aux);
    switch(ga.param){
        case ID:
            sprintf(def,"A XOR E XOR F");
            break;
        case KEY1:
            sprintf(def,"NOT(A XOR F)");
            break;
        case KEY2:
            sprintf(def,"A XOR F");
            break;
    }
    printf("a4 (%s):0x%s\n",def,aux);
    
    toHexString(ga.a5,aux);
    switch(ga.param){
        case ID:
            sprintf(def,"B XOR D XOR E");
            break;
        case KEY1:
            sprintf(def,"NOT(B XOR D)");
            break;
        case KEY2:
            sprintf(def,"B XOR D");
            break;
    }
    printf("a5 (%s):0x%s\n",def,aux);
    
    toHexString(ga.a6,aux);
    switch(ga.param){
        case ID:
            sprintf(def,"D XOR E XOR F");
            break;
        case KEY1:
            sprintf(def,"B XOR F");
            break;
        case KEY2:
            sprintf(def,"NOT(B XOR F)");
            break;
    }
    printf("a6 (%s):0x%s\n",def,aux);
    
    toHexString(ga.a7,aux);
    switch(ga.param){
        case ID:
            sprintf(def,"NOT(A XOR B XOR D XOR E)");
            break;
        case KEY1:
            sprintf(def,"A XOR B XOR D");
            break;
        case KEY2:
            sprintf(def,"A XOR B XOR D");
            break;
    }
    printf("a7 (%s):0x%s\n",def,aux);
    
    toHexString(ga.a8,aux);
    switch(ga.param){
        case ID:
            sprintf(def,"A XOR D XOR E XOR F");
            break;
        case KEY1:
            sprintf(def,"A XOR B XOR F");
            break;
        case KEY2:
            sprintf(def,"A XOR B XOR F");
            break;
    }
    printf("a8 (%s):0x%s\n",def,aux);
    
}



//------------------------------------------------------------------------------
// APPROXIMATION BUILDERS (ID^,K1^,K2)
//------------------------------------------------------------------------------
GA_ID buildID_(leakedSession ls){
    int96 ops [] = {ls.A,ls.B,ls.D,ls.E};
    int96 ops2[] = {ls.A,ls.D,ls.E,ls.F};
    GA_ID ga_out = {
                        .a1 = not(xor(ls.E,ls.F)),
                        .a2 = xor3(ls.A,ls.B,ls.E) , 
                        .a3 = xor3(ls.A,ls.D,ls.E),
                        .a4 = xor3(ls.A,ls.E,ls.F),
                        .a5 = xor3(ls.B,ls.D,ls.E),
                        .a6 = xor3(ls.D,ls.E,ls.F),
                        .a7 = not(xorN(4,ops)),
                        .a8 = xorN(4,ops2),
                        .param = ID
                                                        };
    return ga_out;
}
GA_K1 buildK1_(leakedSession ls){
    GA_K1 ga_out = {
                    //a1:K1 ~= D
                        .a1 = ls.D,
                    //a2:K1~=F
                        .a2 = ls.F , 
                    //a3:K1~= A xor D
                        .a3 = xor(ls.A,ls.D),
                    //a4: K1~= not(A xor F)
                        .a4 = not(xor(ls.A,ls.F)),
                    //a5: K1~= not(B xor D)
                        .a5 = not(xor(ls.B,ls.D)),
                    //a6: K2~= B xor F
                        .a6 = xor(ls.B, ls.F),
                    //a7: K1~= A xor B xor D
                        .a7 = xor3(ls.A,ls.B,ls.D),
                    //a8: K1~= A xor B xor F
                        .a8 = xor3(ls.B,ls.B,ls.F),
                        .param = KEY1
                                                        };
    return ga_out;
}
GA_K2 buildK2_(leakedSession ls){
    GA_K2 ga_out = {
                    //a1:K2 ~= D
                        .a1 = ls.D,
                    //a2:K2~=F
                        .a2 = ls.F , 
                    //a3:K2~= not(A xor D)
                        .a3 = not(xor(ls.A,ls.D)),
                    //a4: K2~= A xor F
                        .a4 = xor(ls.A,ls.F),
                    //a5: K2~= B xor D
                        .a5 = xor(ls.B,ls.D),
                    //a6: K2~= not (B xor F)
                        .a6 = not(xor(ls.B, ls.F)),
                    //a7: K2~= A xor B xor D
                        .a7 = xor3(ls.A,ls.B,ls.D),
                    //a8: K2~= A xor B xor F
                        .a8 = xor3(ls.A,ls.B,ls.F),
                        .param = KEY2
                                                        };
    return ga_out;
}


// [TESTED] Counts the number of different bits present in two numbers
unsigned int hammingDistance(int96 op1,int96 op2){
    int96 xoredPos = xor(op1,op2);
    unsigned int hd = 0;
    unsigned int current = 0;
    for(int bitIterator = 0;     bitIterator<96; bitIterator++){
        if(bitIterator  <32){//ls part, shift right and mask to get everything from lsB
           current = (unsigned int) ((unsigned long) 1) & (xoredPos.lsB>>bitIterator) ;
           //printf("Bit [%d] %u \n",bitIterator,current);
           if(current==1) hd++;
        }
        else{//ms part
            current =  ((unsigned int) 1) & (xoredPos.msB>>(bitIterator-32)) ;
            //printf("Bit [%d] %u\n",bitIterator,current);
           if(current==1) hd++;
        }
    }
    return hd;
}

void printLeakedSession(leakedSession ls){
    printf("\tpid2:"); printHex(ls.pid2);
    printf("\tA:"); printHex(ls.A);
    printf("\tB:"); printHex(ls.B);
    printf("\tD:"); printHex(ls.D);
    printf("\tE:"); printHex(ls.B);
    printf("\tF:"); printHex(ls.F);
}



/*Checked*/void David_Prasad_Round(tag * t, reader * r, leakedSession * adv){
        adv->pid2 = sendPseudonym2(t);
        adv->pid = sendPseudonym(t);
        //testAssertionPrint("Correct eavesdrop",int96Equals(adv->pid2,t->pid2),NULL);
        //printProtocolState(t,r);
        int96 copyn1, copyn2;
        copyn1 = r->n1;
        copyn2 = r->n2;
        adv->A = sendA((*r),t);
        if(!int96Equals(t->n1,r->n1)) {printf("[Error] in Exchange 'A': n1 is different in T and R\n");return;}
        adv->B = sendB((*r),t);
        if(!int96Equals(t->n2,r->n2)) {printf("[Error] in Exchange 'B': n2 is different in T and R\n");return;}
        adv->D = sendD(r,t);
        if(!int96Equals(t->D,r->D)) {printf("[Error] in Exchange 'D': D' is different in T\n");return;}
        adv->E = sendE((*t),r);
        if(!int96Equals(r->id,t->id)){printf("[Error] in Exchange 'E': ID is different\n");return;}
        if(!int96Equals(t->k1,r->k1)) {printf("[Error] in Exchange 'E': K1 is different\n");return;}
        if(!int96Equals(t->k2,r->k2)) {printf("[Error] in Exchange 'E': K2 is different\n");return;}
        adv->F = sendF(t,r);
        if(int96Equals3(t->n1,r->n1,copyn1)) { printf("[Error] n1 not correctly updated.\n");}
        if(int96Equals3(t->n2,r->n2,copyn2)) { printf("[Error] n2 not correctly updated.\n");}
}

void printColumnCountVector(unsigned int * cCount ,unsigned int sessID){
    printf("Column count vector SESSION %u\n (LSB)[", sessID);
    for(int i = 0 ; i<96; i++){
        printf("%u",cCount[i]); if(i!=(96-1)){ printf(",");}
    }
    printf("](MSB)\n");
}

/* Counts the number of ones per bit column, returns an array of 96 positions (representing each bit column),
*  holding the the number of ones found in such column. Bits are stored in the position they belong logically using array indexes
* that is, pos[0] contains the lsb, pos[95] is msb count
*/
bool int96_columnCount( int96 * ops, unsigned int nOps, unsigned int * resultCount   ){

    if(!ops||!resultCount) {printf("ERROR! int96_columnCount: pointers to function can't be null\n"); return false;}

    for(int bitIt=0; bitIt<96; bitIt++){ //Bit by bit compute the count
        unsigned int column_i_count = 0 ;
        
        for(int opIt=0; opIt<nOps; opIt++){//For each operator involved (that is, for each approximation{a1,a2...}), we compute the number of ones
            
            unsigned long bitI = 0;
            char hexOp [24];
            toHexString(ops[opIt],hexOp);
            //printf(" - int96_columnCount Operator %d: 0x%s \n", opIt, hexOp);
            if(bitIt<32) //If computing the least significant part 
            {   
                bitI  = ops[opIt].lsB & ((unsigned int) 1<<bitIt) ;
                if(bitI>=1){ column_i_count++;}
            }
            else{
                bitI  = ops[opIt].msB & ((unsigned long) 1<<(bitIt%32)) ;
                if(bitI>=1){ column_i_count++;}
            }
        }
        //printf("bit [%d] = %u\n",bitIt,column_i_count);
        resultCount[bitIt] = column_i_count; //We assign the counter of ones per column to the position on the result array;
        //printf("bitCount[%d] = %u\n",bitIt,resultCount[bitIt]);
    }
    //printColumnCountVector(resultCount, 666);
    return true;
}
/*Computes a global sum for all the eavesdropped sessions provided and 
 * creates the final approximation based on the delta passed as argument */
int96 bitwiseSumOfSessionApproximations(struct goodApproximations * approximations, unsigned int nEavesdroppedSessions , float deltaThreshold){
        //Per session I can compute an array of column counts, which I have to add together
        unsigned int globalPerColumnCount [96];
        memset(globalPerColumnCount,0,sizeof(unsigned int)*96);

        /* We traverse the set of eavesdropped session approximations (8 approximations per protocol round) */ 
        for(int appxIt= 0 ; appxIt<nEavesdroppedSessions; appxIt++){
            //Compute bitwise Sum of a single session  
            unsigned int sessColumnCount [96];
            memset(sessColumnCount,0,sizeof(unsigned int)*96);

            int96 aux [] = {   
                    approximations[appxIt].a1,
                    approximations[appxIt].a2,
                    approximations[appxIt].a3,
                    approximations[appxIt].a4,
                    approximations[appxIt].a5,
                    approximations[appxIt].a6,
                    approximations[appxIt].a7,
                    approximations[appxIt].a8
            };
            /* UNCOMMENT TO SEE APPROXIMATION SET FOR EACH EAVESDROPPED SESSION */
            //printf("\n\n----------------------------------------------\n");
            //printApproximationSet(approximations[appxIt],appxIt);
            //printf("----------------------------------------------\n\n");
            /* ---------------------------------------------------------------- */
            if(!int96_columnCount(aux,8,sessColumnCount)) {printf("ERROR! column count appxIt[%d]\n",appxIt);}
            //printColumnCountVector(sessColumnCount,appxIt);

            // HERE !!!!!!!! Update global count based on current local
            for(int updaterGC = 0 ; updaterGC<96; updaterGC++ ){
                if(sessColumnCount[updaterGC] != 0){
                    globalPerColumnCount[updaterGC] +=  sessColumnCount[updaterGC];
                }
            }
          }



        int96 finalApproximation = rand96();

        //Decide final value of approximation using provided delta
        for(int bitIter= 0 ; bitIter<96; bitIter++){
            if(globalPerColumnCount[bitIter]>=  deltaThreshold){ 
                //printf("vec[%d] = %u i> %.4f so set bit[%d] to 1\n",bitIter,globalPerColumnCount[bitIter],deltaThreshold,bitIter );
                setBit(&finalApproximation,bitIter,true);
            }
            else{
                setBit(&finalApproximation,bitIter,false);
            }
        }
    return finalApproximation;

}


int main(int argc, char ** argv){


    
    
    /* Initialisation values for the protocol */
    tag t;
    reader r;
    int96 myID = { 0x616c656a616e6472, 0x6f726579 }; //ID = alejandrorey
    int96 pid_0 = {0x6d69706964313233, 0x34353637}; //PID = mipid1234567
    int96 pid2_0 = {0x69616d6e6f74616c, 0x65787878}; //PID2 = iamnotalexxx
    int96 k1 = { 0x3132333435363738 , 0x39313030 }; // K1 = 123456789100
    int96 k2 = { 0x61646d696e61646d, 0x696e3030  }; // K2 = adminadmin00
    /* Obtain the number of sessions to eavesdrop */
    if(argc<2 || argc>3){ printf("David Prasad's Cryptanalysis requires: <#sessionsToEavesdrop> (<useRotationsFlag(0/1)>)\n"); return -1;}
    unsigned int N_eavesdrop = atoi(argv[1]); 
    if(argc == 3) USE_ROTATE = atoi(argv[2]);
    if(N_eavesdrop>=MAX_EAVESDROPPED){printf("You asked for too many eavesdropped sessions: <#sessionsToEavesdrop> should be lower than %d",MAX_EAVESDROPPED); return -1;} 

    /* We create an array de leaked sessions (to store all the corresponding messages for session "i")*/
    leakedSession adversary [MAX_EAVESDROPPED];

    /* Create structures to hold good approximations (each position refers to a given round of the protocol) 
     * for each session, holds approximations 1,2,3...n  */
    GA_K1 apprK1 [MAX_EAVESDROPPED];
    GA_K2 apprK2 [MAX_EAVESDROPPED];
    GA_ID apprID [MAX_EAVESDROPPED];
    
    /* Run the protocol N times and gather/store all the leaked info */
    Init_David_Prasad(myID , pid_0, pid2_0 , k1, k2, &t, &r);
    printf("Eavesdropping BEGAN ...\n");
    for(int i = 0 ; i < N_eavesdrop; i++){
        David_Prasad_Round(&t,&r, &(adversary[i]));
        printf("\tEavesdropped session %d \n", i );
        if(i>0) printf("\t\tpid(t+1) == pid2(t)? %s \n", int96Equals(adversary[i].pid,adversary[i-1].pid2)? "true": "false");
        apprID[i] = buildID_(adversary[i]);
        apprK1[i] = buildK1_(adversary[i]);
        apprK2[i] = buildK2_(adversary[i]);
    }
    printf("Eavesdropping COMPLETE !\n");


    int nApproximations = 8;

    
    //[LAST STEP] Cryptanalysis:
    //- setThreshold to = 0.5 * <numApproximations> * <#eavesdropped>
    /* Threshold usado para determinar el valor de un parámetro del protocolo: say X*/
    float thresholdDelta_paramX = 0.5f * nApproximations * N_eavesdrop ;
    //- Compute columnwise Addition //- Set final value of guessing as: 
    //                      0 if addition is > thresholdDelta
    //                      1 if addition is < thresholdDelta
    char idApprx_str [24]; char k1Apprx_str [24]; char k2Apprx_str [24];
    char idStr [24]; char k1Str [24]; char k2Str [24];
    int96 id_finalApprox = bitwiseSumOfSessionApproximations(apprID ,N_eavesdrop,thresholdDelta_paramX);
    int96 k1_finalApprox = bitwiseSumOfSessionApproximations(apprK1 ,N_eavesdrop,thresholdDelta_paramX);
    int96 k2_finalApprox = bitwiseSumOfSessionApproximations(apprK2 ,N_eavesdrop,thresholdDelta_paramX);
    toHexString(myID,idStr);
    toHexString(k1,k1Str);
    toHexString(k2,k2Str);
    toHexString(id_finalApprox,idApprx_str);
    toHexString(k1_finalApprox,k1Apprx_str);
    toHexString(k2_finalApprox,k2Apprx_str);
    // Compare "Approximation" to the actual value using Hamming Distance to obtain number of bits disclosed
    printf("--------------------------------------------------------------------------\n");
    printf("-----------------CRYPTANALYSIS RESULT-------------------------------------\n");
    printf("--------------------------------------------------------------------------\n");
    printf(" #Eavesdropped sessions: %d  \n",N_eavesdrop);
    switch (USE_ROTATE)
    {
        case 0:
            printf(" Using David-Prasad AS IS \n");
            break;
        case 1:
            printf(" Using MODIFIED David-Prasad %d ROTATION1 (message D ) \n",USE_ROTATE);
            break;
        case 2:
            printf(" Using MODIFIED David-Prasad %d ROTATIONS (messages D and E) \n",USE_ROTATE);
            break;
        default : 
            printf(" Using MODIFIED David-Prasad %d ROTATIONS (messages D and E) \n",USE_ROTATE);
    }
    
    printf(" Threshold = %.4f \n\n", thresholdDelta_paramX);
    printf(" [Guessed ID] 0x%s\n [Actual  ID] 0x%s\n\tHamming distance = %u => correctly recovered bits = %u \n\n",idApprx_str,idStr,hammingDistance(myID,id_finalApprox),96-hammingDistance(myID,id_finalApprox));
    printf(" [Guessed k1] 0x%s\n [Actual  k1] 0x%s\n\tHamming distance = %u => correctly recovered bits = %u \n\n",k1Apprx_str,k1Str,hammingDistance(k1,k1_finalApprox),96-hammingDistance(k1,k1_finalApprox));
    printf(" [Guessed k2] 0x%s\n [Actual  k2] 0x%s\n\tHamming distance = %u => correctly recovered bits = %u \n\n",k2Apprx_str,k2Str,hammingDistance(k2,k2_finalApprox),96-hammingDistance(k2,k2_finalApprox));
    printf("--------------------------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------\n");
    //test_int96(false);
  


    return 0;


    
}

/*printf(" size of type:  char %ld byte/s; %ld bits \n",sizeof(char), sizeof(char)*8);
printf(" size of type:  short %ld byte/s; %ld bits \n",sizeof(short), sizeof(short)*8);
printf(" size of type: unsigned int %ld byte/s; %ld bits \n",sizeof(unsigned int), sizeof(unsigned int)*8);
printf(" size of type: int %ld byte/s; %ld bits \n",sizeof(int), sizeof(int)*8);
printf(" size of type: long int %ld byte/s; %ld bits \n",sizeof(long int), sizeof(long int)*8);
printf(" size of type: long long int %ld byte/s; %ld bits \n",sizeof(long long int), sizeof(long long int)*8);
printf(" size of type: unsigned long %ld byte/s; %ld bits \n",sizeof(unsigned long), sizeof(unsigned long)*8);

printf(" size of type: int96 %ld byte/s; %ld bits \n",sizeof(a.msB)+sizeof(a.lsB), (sizeof(a.msB)+sizeof(a.lsB))*8);*/