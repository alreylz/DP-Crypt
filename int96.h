/* int96:
*   Implementation of a 12 Bytes (96-bit) lenght integer number
*   Developed by coredamnwork (https://github.com/coredamnwork)
*/
#define SIZE_INT96_B 12 //Defines the size in bytes of the custom datatype
#define SIZE_INT96_b 12*8 //Defomes the size in bits of the custom datatype
#define TESTING_int96 0
typedef struct {
	unsigned long msB; // size = 8 bytes = 64 bits
	unsigned int lsB; // size = 4 bytes = 32 bits
} int96;    

// INITIALISATION
/*[OK]*/int96 ones();
/*[OK]*/int96 zero();
/*[OK]*/void setRand96_Seed(unsigned int seed);
/*[OK]*/int96 rand96();

//OPERATIONS
    //Equality ( op1 == op2 )
/*[OK]*/bool int96EqualsPtr(int96 * op1, int96 * op2);
/*[OK]*/bool int96Equals(int96 op1, int96 op2);
/*[OK]*/bool int96Equals3(int96 op1, int96 op2, int96 op3);
    //BITWISE
/*[OK]*/int96 and( int96 op1 , int96 op2);
/*[OK]*/int96 and3( int96 op1 , int96 op2, int96 op3);
/*[OK]*/int96 xor( int96 op1 , int96 op2);
/*[OK]*/int96 xor3( int96 op1 , int96 op2, int96 op3);
/*[OK]*/int96 xorN( unsigned int n , int96 * ops);
/*[OK]*/int96 or( int96 op1 , int96 op2);
/*[OK]*/int96 not (int96 op1);

// DEBUGGING
/*[OK]*/void toHexString(int96 toPrint, char out [] );
/*[OK]*/void printHex(int96 toPrint);
/*[OK]*/void printBits(int96 toPrint);
/*[OK]*/bool testAssertionPrint( const char * label, bool condition, int * testID );


