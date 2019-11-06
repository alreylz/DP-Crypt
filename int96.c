#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <string.h>

#include "int96.h"
// --------------------------------------------------------
// -------------------INITIALISATION-----------------------
// --------------------------------------------------------
//Creates an int of 12 bytes (96 bits; 24 0S en HEX)
int96 zero(){
    int96 out = {0,0};
    return out;
}
//Creates an int of 12 bytes (96 bits; 24 1s en HEX)
int96 ones(){
    int96 allOnes = zero();
        allOnes.msB--;
        allOnes.lsB--;
    return allOnes;
}
//Initialises a the randomization algorithm to generate 12 byte integer numbers
void setRand96_Seed(unsigned int seed){
    srand(seed);
    //srand(time(0)); to initialise with current time
}
//Generates a random number of 96 bits
int96 rand96(){
    //Each part of the int
    unsigned long randMsb = 0;
    randMsb = (unsigned long) ((unsigned long)rand()<<32) | (unsigned long)rand();
    
    int96 out = {.msB = randMsb , .lsB = rand() };
    return out;
    
}
// --------------------------------------------------------
// -------------------COMPARISON---------------------------
// --------------------------------------------------------
bool int96EqualsPtr(int96 * op1, int96 * op2){
    if(op1->msB != op2->msB) return false;
    if(op1->lsB != op2->lsB) return false;
    return true;
}
bool int96Equals(int96 op1, int96 op2){
    if(op1.msB != op2.msB) return false;
    if(op1.lsB != op2.lsB) return false;
    return true;
}
bool int96Equals3(int96 op1, int96 op2, int96 op3){
    if(op1.msB != op2.msB || op2.msB || op3.msB || op1.msB != op3.msB ) return false;
    if(op1.lsB != op2.lsB || op2.lsB != op3.lsB || op1.lsB != op3.lsB) return false;
    return true;
}

// --------------------------------------------------------
// ----------------BITWISE OPERATORS-----------------------
// --------------------------------------------------------

//AND 
int96 and( int96 op1 , int96 op2){
    
    int96 out = { .msB = (op1.msB & op2.msB) , .lsB = (op1.lsB & op2.lsB) }  ;
    return out;
}
int96 and3( int96 op1 , int96 op2, int96 op3){
    
    int96 out = { .msB = (op1.msB & op2.msB & op3.msB) , .lsB = (op1.lsB & op2.lsB & op3.lsB) }  ;
    return out;
}
// XOR 
int96 xor( int96 op1 , int96 op2){
    int96 out = { .msB = (op1.msB ^ op2.msB) , .lsB = (op1.lsB ^ op2.lsB) };
    return out;
}
int96 xor3( int96 op1 , int96 op2, int96 op3){
    int96 out = { .msB = (op1.msB ^ op2.msB ^ op3.msB) , .lsB = (op1.lsB ^ op2.lsB ^ op3.lsB) };
    return out;
}
int96 xorN( unsigned int n , int96 * ops){
    int96 out;
    if(! n>1 && !ops){ 
        printf("xorN Error\n");
        return zero();
    }
    out = ops[0]; // We start from the value of the first operator
    for(unsigned int i=1 ; i<n ; i++){ //Compute xor consecutively
            out.msB = out.msB ^ ops[i].msB;
            out.lsB = out.lsB ^ ops[i].lsB;
    }
    return out;
}
// OR
int96 or( int96 op1 , int96 op2){
    
    int96 out = { .msB = (op1.msB | op2.msB) , .lsB = (op1.lsB | op2.lsB) }  ;
    return out;
}
// NOT
int96 not (int96 op1){
    int96 out = {.msB = ~(op1.msB), .lsB = ~(op1.lsB) };
    return out;
}
bool getBit(int96 toRead, int pos){

    unsigned long maskedValue = 3;
    if(pos>=96 || pos<0) printf("[Error] getBit pos[%d]\n",pos);
    else if(pos > 31){ //Look within msB
        maskedValue = 0UL;
        //maskedValue |= ( 1UL<< (unsigned long) pos);
        //maskedValue = toRead.msB & maskedValue;
        maskedValue = toRead.msB>>(pos-32) & 1UL ;
    }
    else if(pos<32 && pos>=0){//Look within lsB
        maskedValue = 0;
        // maskedValue |= ((unsigned int) 1<<pos);
        // maskedValue = (unsigned long) toRead.lsB & maskedValue;
        maskedValue = toRead.lsB >> pos & 1UL ;
    }

    if(maskedValue >= 1) return true; // a 1 at position pos
    else if(maskedValue==0) return false;
    else{ printf("[Error getBit pos[%d] (Should never happen)\n",pos); return false; }

}


/*[OK]*/int96 setBit(int96 * toChange,  int pos, bool value){
            unsigned long binaryValue = 1 ;
            unsigned int binaryValueUI = 1;
            if(pos>=96 || pos<0){ printf("SetBit ERROR\n"); return ones();}

            if(pos<32 && pos>=0){
                unsigned int mask;
                
                if(value == true ){
                    //printf("MASK to set bit [%d]=1 is %u\n",pos, mask );
                    mask = ((unsigned int) binaryValueUI<<pos);
                    toChange->lsB = (toChange->lsB) | mask  ;
                } 
                else{ 
                    mask = ~((unsigned int) binaryValueUI<<pos); 
                    toChange->lsB = (toChange->lsB) & mask  ;
                }

            }
            else if (pos>=32 && pos<96) {   /* FUCKING ERROR IS HERE */
                unsigned long mask = 0;
                
                if(value == true){
                    mask |=  (unsigned long) (1UL<<(pos-32UL));
                    unsigned long negMask = ~ mask;
                    //printf("MASK to set bit [%d]=1 is %016lx , neg=%016lx  \n",pos, mask,negMask );
                    toChange->msB = ((toChange->msB) & negMask) | mask; 
                }
                else{
                    mask =  (unsigned long) ~(1UL<<(pos-32UL));
                    //printf("%016lx  (MASK to set bit [%d]=0 ) \n",mask,pos );
                    //printf("%016lx\n",toChange->msB);
                    //printf("%016lx\n",mask & toChange->msB);
                    toChange->msB = toChange->msB & mask; 
                }
            }
            return (*toChange);
        }




bool rotateBits(int96 * toRotate, int rotation_bits){
    
    int96 copy = {.msB = toRotate->msB, .lsB = toRotate->lsB};

    if(rotation_bits>0) // rot -->
    {
        //printf("RR (%d bits) \n", rotation_bits);
        //Traverse copy and reassign data of toRotate
        for(int bitIt=0; bitIt<SIZE_INT96_b; bitIt++){
            int newPos = (bitIt-rotation_bits)%SIZE_INT96_b;
            if(newPos<0) newPos= newPos + SIZE_INT96_b;    
            //printf("pos0 [%d]=%s ---> pos'[%u]=%s \n",bitIt, getBit(copy,bitIt)? "1":"0" ,newPos, getBit((*toRotate),(unsigned int) newPos)? "1":"0" );
            setBit(toRotate,newPos,getBit(copy,bitIt));
        }

    }
    else if (rotation_bits<0)
    { //rot <--
        //printf("RL (%d bits) \n", rotation_bits);
        //Traverse copy and reassign data of toRotate
        for(int bitIt=0; bitIt<96; bitIt++){
            int newPos = (bitIt+abs(rotation_bits)) % 96;
            while(newPos<0) newPos+=96;
            newPos = newPos%96;
            //printf("pos [%d] ---> pos [%d] \n",bitIt,newPos);
            setBit(toRotate,newPos ,getBit(copy, bitIt));
        }
    }
    return true;

}
// --------------------------------------------------------
// ------------------------DEBUG---------------------------
// --------------------------------------------------------

void printBits(int96 toPrint){

    char * bitString  = calloc(97, sizeof(char) );
    //Null terminate string
    bitString[96] = '\0';
    int insert;
    for(int i=95,insert=0; i>=0, insert<96; i--, insert++){
        unsigned long bit = 0;
        if(i>31){ //Si msB
            bit =  toPrint.msB & ( (unsigned long) 1<<(i-32));
            //printf("Bit[%d]=%lu\n", i, bit);
            if(bit > 0) bitString[insert] = '1';
            else{ bitString[insert] = '0'; }
        }
        else{   //Si lsB
            bit = toPrint.lsB & ((unsigned long) 1<<i);
            if(bit > 0) bitString[insert] = '1';
            else{ bitString[insert] = '0'; }
        }
        //printf("iteration %d, bit %c\n", i,bitString[i]);
    }
    
    int i; // Legend for display
    printf("| MSB  |");
    for(int s=0; s<= (12*9 -(2*9)) ;s+=1) printf(" ");
    printf("| LSB  |\n");
    for( i = 11 ; i > 9; i-- ) printf("<--%d--> ",i);
    for( ; i>=0; i--) printf("<--%d---> ",i);
    printf("\n");

    for(int i = 0; i<97; i++) { if(i%8 ==0 && i>1){printf(" ");} printf("%c", bitString[i]);}
    printf("\n");

    //printf("%s (v2:stringwise)\n", bitString);
    free(bitString);
}
void toHexString(int96 toPrint, char out [] ){
    // We need 24 HEX digit to represent  64+32 = 96 bits
    sprintf(out, "%016lx%08x", toPrint.msB, toPrint.lsB);
}
void printHex(int96 toPrint){
    // We need 24 HEX digit to represent  64+32 = 96 bits
    char * hexString  = calloc(25, sizeof(char) );
    hexString[24] = '\0';
    sprintf(hexString, "%016lx%08x", toPrint.msB, toPrint.lsB);
    printf("0x%s\n", hexString);
    free(hexString);
}

// --------------------------------------------------------
// ----------------------TESTING---------------------------
// --------------------------------------------------------

bool testAssertionPrint( const char * label, bool condition, int * testID ){
    if(testID == NULL){
        printf("Test (%s) [%s]\n",label,condition? "OK": "!");
        return condition;
    }
        (*testID)++; 
        printf("Test %d (%s) [%s]\n", (*testID),label,condition? "OK": "!");
        return condition;
}
void test_int96(bool resultPrints){

    int testID = 0;
    printf("-------------------------------------------------------------\n");
    printf("----------------INT96 TEST EXECUTION STARTED------------------\n");
    printf("-------------------------------------------------------------\n\n");
    //TEST CASE 1 (ones)
    int96 allOnes = ones();
    int96 manualAllOnes = {.msB = 0xFFFFFFFFFFFFFFFF, .lsB = 0xFFFFFFFF };
    if(resultPrints) printBits(allOnes);
    testAssertionPrint("allOnes",int96Equals(allOnes,manualAllOnes), &testID); 

    //TEST CASE 2 (zero)
    int96 allZeros = zero();
    int96 manualAllZeros = { .msB = 0x0000000000000000, .lsB = 0x00000000 };
    if(resultPrints) printBits(allZeros);
    testAssertionPrint("allZeros",int96Equals(allZeros,manualAllZeros), &testID);


    //TEST CASE 3 (rand96)
    setRand96_Seed(4);
    int96 n1 = rand96();
    setRand96_Seed(4);
    int96 n2 = rand96();
    testAssertionPrint("rand96",int96Equals(n1,n2),&testID);
    n2 = rand96();
    testAssertionPrint("rand96[expected not equal]",!int96Equals(n1,n2),&testID);
    testAssertionPrint("rand96Ptr", !int96EqualsPtr(&n1,&n2), &testID);
    
    //TEST CASE 4 AND
    int96 ej1 = { .msB = 0x11111111111111FF, .lsB = 0x33333333 };
    int96 ej2 = { .msB = 0x1111111111111100, .lsB = 0x22222222 };
    int96 ej1_AND_ej2 = { .msB = 0x1111111111111100, .lsB = 0x22222222 };
    testAssertionPrint("and",int96Equals(and(ej1,ej2),ej1_AND_ej2), &testID);
    int96 ej3 = rand96();
    testAssertionPrint("and3",int96Equals(and3(ej1,ej2,ej3),and(ej1_AND_ej2,ej3)), &testID);
    
    //TEST CASE 5 XOR
    int96 ej4 = { 0x1122334455667788, 0x99101112};
    int96 xoredAllOnes3j4 = {0xeeddccbbaa998877,0x66efeeed};
    testAssertionPrint("xor", int96Equals(xor(allOnes,allZeros),allOnes),&testID);
    testAssertionPrint("xor(any number)", int96Equals( xor(allOnes,ej4),xoredAllOnes3j4), &testID) ;
    testAssertionPrint("xor3(identity)",int96Equals(xor3(allZeros,allOnes,allZeros),allOnes), &testID );
    testAssertionPrint("xor3(other)",int96Equals(xor3(allZeros,allOnes,ej4),xoredAllOnes3j4), &testID );
    int96 arrToXOR [] = {ej4,ej1,ej2,ej3,ej1,ej2,ej3};
    testAssertionPrint("xorN",int96Equals(xorN(7,arrToXOR),ej4),&testID);
    
    //TEST CASE 6 OR
    int96 aaaS = {0xAAAAAAAAAAAAAAAA, 0xAAAAAAAA};
    int96 fives = {0x5555555555555555, 0x55555555};
    testAssertionPrint("or",int96Equals(or(aaaS,fives),allOnes), &testID);
    int96 ej1_OR_ej2 = { .msB = 0x11111111111111FF, .lsB = 0x33333333 };
    testAssertionPrint("or(any)",int96Equals(or(ej1,ej2),ej1_OR_ej2), &testID);

    //TEST CASE 7 NOT
    testAssertionPrint("not",int96Equals(not(aaaS),fives), &testID);

    char assString  [20];
    toHexString(aaaS,assString);
    //printf("Printing all As : 0x%s [OK]\n",assString);


    //TEST CASE 8 SET BIT
    int96 toSetBit = zero();
    for(int i=0; i<96; i++){
        setBit(&toSetBit,i,true);
    }
    testAssertionPrint("setBit", int96Equals(toSetBit,ones()),&testID);
    setBit(&toSetBit,32,false);
    int96 expectedResultSetBits = {0xFFFFFFFFFFFFFFFE, 0xFFFFFFFF};
    testAssertionPrint("setBit(2) pos 32", int96Equals(toSetBit,expectedResultSetBits),&testID);
    int96 expectedResultSetBits2 = {.msB = 0xFFFFFFFFFFFFFFFE, .lsB = 0x7FFFFFFF };
    setBit(&toSetBit,31,false);
    testAssertionPrint("setBit(3) pos 31", int96Equals(toSetBit,expectedResultSetBits2),&testID);
    int96 toSetBit2 = ones();
    bool value = true;
    for(int i=32; i<63; i++){
            setBit(&toSetBit2,i,value);
            if(getBit(toSetBit2,i)!=value){printf("Error pos[%d] written=%s vs intended=%s\n",i,getBit(toSetBit2,i)? "1":"0",value?"1":"0" );}
            else{
                //printf(" pos[%d] written=%s vs intended=%s\n",i,getBit(toSetBit2,i)? "1":"0",value?"1":"0" );
            }
            if(value==true) value = false;
            else value = true;
    
    }
    printBits(toSetBit2);

    for(int i=95;i>=0;i--){
        printf("%s", getBit(toSetBit2,i)?"1":"0");
    }
    printf("\n");

    //TEST CASE 9 ROTATE 
    int96 exampleRot = {.msB = 0x00000000000000FF, .lsB = 0x000000FF};
    printf("Before Rotation\n");
    printBits(exampleRot);
    //Test rotation right
    int96 exampleAfterRotRight = {.msB = 0xFF00000000000000, .lsB = 0xFF000000};
    rotateBits(&exampleRot,8);
    printf("After Rotation\n");
    printBits(exampleRot);
    printf("Expected \n");
    printBits(exampleAfterRotRight);
    testAssertionPrint("Right rotation (8 bits)",int96Equals(exampleRot, exampleAfterRotRight), &testID);
    


    //Test rotation left
    int96 exampleAfterRotLeft = {0x000000000000FF00,0x0000FF00};
    rotateBits(&exampleRot,-8);
    rotateBits(&exampleRot,-8);
    testAssertionPrint("Left rotation",int96Equals(exampleRot, exampleAfterRotLeft), &testID);
    
    
    int96 hugeRotation ={0x0000000000000000,0x00000001};
    int96 hugeRotation2 =  hugeRotation;
    rotateBits(&hugeRotation,-95);
    int96 hugeRotationLeftResult ={0x8000000000000000,0x00000000};
    testAssertionPrint("Huge Left rotation",int96Equals(hugeRotation, hugeRotationLeftResult), &testID);
    printHex(hugeRotation);
    rotateBits(&hugeRotation2,95);
    int96 hugeRotationRightResult ={0x0000000000000000,0x0000002};
    testAssertionPrint("Huge Right rotation",int96Equals(hugeRotation2, hugeRotationRightResult), &testID);
    printHex(hugeRotation2);
    

}

#if TESTING_int96 == 1

int main(int argc , char ** argv){
    test_int96(false);
    return 0;
}

#endif 