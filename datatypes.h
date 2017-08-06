/*******************************************************************************
*                        Oliver Fádi Amairi                                    *
*                                                                              *
*                                                                              *
*                                                                              *
*                                                                              *
*******************************************************************************/


/*******************************************************************************
*                                  Defines                                     *
*******************************************************************************/
#ifndef DATATYPES
#define DATATYPES

//Bool variables
#define TRUE 1
#define FALSE 0


//Bitmasks
#define MBIT1 (0x00000001)
#define MBIT2 (0x00000002)
#define MBIT3 (0x00000004)
#define MBIT4 (0x00000008)
#define MBIT5 (0x00000010)
#define MBIT6 (0x00000020)
#define MBIT7 (0x00000040)
#define MBIT8 (0x00000080)
#define MBIT9 (0x00000100)
#define MBIT10 (0x00000200)
#define MBIT11 (0x00000400)
#define MBIT12 (0x00000800)
#define MBIT13 (0x00001000)
#define MBIT14 (0x00002000)
#define MBIT15 (0x00004000)
#define MBIT16 (0x00008000)
#define MBIT17 (0x00010000)
#define MBIT18 (0x00020000)
#define MBIT19 (0x00040000)
#define MBIT20 (0x00080000)
#define MBIT21 (0x00100000)
#define MBIT22 (0x00200000)
#define MBIT23 (0x00400000)
#define MBIT24 (0x00800000)
#define MBIT25 (0x01000000)
#define MBIT26 (0x02000000)
#define MBIT27 (0x04000000)
#define MBIT28 (0x08000000)
#define MBIT29 (0x10000000)
#define MBIT30 (0x20000000)
#define MBIT31 (0x40000000)
#define MBIT32 (0x80000000)



/*******************************************************************************
*                                  Typedefs                                    *
*******************************************************************************/

//the structure of the typedefs(except some exception): t{variable type}{number of bits}{Signed/Unsigned}

//Integer types:
typedef unsigned char tBOOL;
typedef unsigned char tUI8;
typedef char tSI8;
typedef unsigned short int tUI16;
typedef short int tSI16;
typedef unsigned int tUI32;
typedef int tSI32;

//Floating-point types
typedef float tF32;
typedef double tF64;

#endif

