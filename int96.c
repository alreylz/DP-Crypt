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

    char * assString ;
    toHexString(aaaS,assString);
    printf("Printing all As : 0x%s [OK]\n",assString);
}

#if TESTING_int96 == 1

int main(int argc , char ** argv){
    test_int96(false);
    return 0;
}

#endif 
















/*
//[DOESN'T WORK] Allows to set a byte of a given int96 (0: lsb; 15:msb)
bool setByteAt( int96 * toModify , char value , unsigned int pos ){

    int intValue = value -'0';
    //printf("Setting BYTE[%d] if int96 to %d ...\n", pos, intValue);
    //int96 original = (*toModify);
  

    if( pos > SIZE_INT96_B ){ //If the user is trying to set a byte that is outside the range [0-MAX]
        fprintf(stderr, "ERROR: index for replacement is out of bounds %d>%d bytes (int96)\n",pos,SIZE_INT96_B);
        return false;
    }
    else{   
        if(pos>3){ //If trying to set upper part
            unsigned long maskedValue = 0 | ( (unsigned long) intValue<< (pos-4)); //Sets a variable with all zeros but new byte set to new value
            //Clear specific byte and add value
            toModify->msB = (toModify->msB) & ((unsigned long) 0x00 << (pos-4));
            toModify->msB |= maskedValue ;
        }
        else{
            unsigned int maskedValue = 0 | ((unsigned int) intValue << pos);
            //Clear byte
            toModify->lsB &= 0xFFFFFFFF && ((unsigned long)0x00 << pos) ;
            toModify->lsB |= maskedValue ; 
        }
    }
    //printf("Value post modification:\n");
    //printBits((*toModify));
    return true;
}

//[INCOMPLETE] Allows to create an int96 providing a text (in order to make initialisation
// conceptually simple and analogous to a passphrase the user can remember).
int96 strToInt96(const char * str ){

    int strLenght_B = strlen(str); // Length in bytes 
    int96 out = zero();
    //If lenght in bytes (repr. characters in this case) is appropiate (lower than max bytes), convert
    if( strLenght_B > 0 && strLenght_B < SIZE_INT96_B ){
        int unusedBytes = SIZE_INT96_B -  strLenght_B ; //The bytes we won't be setting (in case the string is not 96 bytes)
        for(int byteIt = 0 ; byteIt<strLenght_B; byteIt++){//We convert byte by byte (char by char) letter to numbers
            //out.msB |= str[byteIt] << ( (sizeof(out.msB)*8)-(unusedBytes*8) )

        }
    }
    else{ 
        printf("Incorrect Int96 value created from %s \n",str);
    }

    //out.msB =

    return out;
}
*/
