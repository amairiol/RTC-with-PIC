/*******************************************************************************
*                        Oliver Fádi Amairi                                    *
*            USE DS1302 RTC module with PIC16F877A microcontroller              *
*    To develope this code I used code generator for the timers and I used     *
*            an example code that I found in this link:                        *
*          https://forum.mikroe.com/viewtopic.php?p=70949                      *
*******************************************************************************/


/*******************************************************************************
*                                  Includes                                    *
*******************************************************************************/
#include "DS1302.h"
#include "datatypes.h"

/*******************************************************************************
*                                  Defines                                     *
*******************************************************************************/

/*******************************************************************************
*                           Variables and Function deklarations                              *
*******************************************************************************/

enum eClkRegisters{
    SecondRegister,
    MinuteRegister,
    HourRegister,
    DateRegister,
    MonthRegister,
    DayRegister,
    YearRegister,
    ControlRegister,
    NOF_ClkRegisters
};


extern tUI8 DS1302_CLOCK_DATA[NOF_ClkRegisters];
extern tUI8 DS1302_RAM_DATA[31];



/*******************************************************************************
*  input: -                                                                    *
********************************************************************************
*  output: -                                                                   *
********************************************************************************
*  comment: -                                                                  *
*******************************************************************************/
extern void DS1302_INIT()
{
     DS1302_RST_TRIS = 0; // Define this pin as output
     DS1302_IO_TRIS = 1;  // Define this pin as input for the start, need to change when Write
     DS1302_SCLK_TRIS = 0; // Define this pin as output

     DS1302_RST_DATA = 0;  // Hold DS1302 @ reset mode, int clock still running
     DS1302_SCLK_DATA = 0; // Sclk always begins at low
     DS1302_IO_DATA = 0;   // Hold IO data line low
}

/*******************************************************************************
*  input: -                                                                    *
********************************************************************************
*  output: -                                                                   *
********************************************************************************
*  comment: Read the values form DS1302 clock registers to DS1302_CLOCK_DATA   *
*  array                                                                       *
*******************************************************************************/
extern void DS1302_CLOCK_READ()
{
     tUI8 cc1,cc2,dat;     // cc1 and cc2 are used for counter

     DS1302_IO_TRIS = 0;  // Change IO line to output
     DS1302_RST_DATA = 1; // Release reset line and prepare for transfer

     dat = 0b10111111;    // CLOCK READ BURST COMMAND, <1><R/#C><1><1><1><1><1><R/#W>
     for(cc1=0;cc1<8;cc1++){
         if(dat.F0) DS1302_IO_DATA = 1; else DS1302_IO_DATA = 0; // Output data
         dat = dat >> 1;  // Shift bit one step to the right.

         DS1302_SCLK_DATA = 0;  // Create a clock pulse, about 2us (freq 500Khz)
         delay_us(1);
         DS1302_SCLK_DATA = 1;
         delay_us(1);
         DS1302_SCLK_DATA = 0;
     }
     //-------------------------------------------------------------------------
     // After sending command, next is to read all 8 bytes of data
     //-------------------------------------------------------------------------
     DS1302_IO_TRIS = 1;  // Change IO line to Input, to receive data

     for(cc2=0;cc2<8;cc2++){
         for(cc1=0;cc1<8;cc1++){
             DS1302_SCLK_DATA = 0;  // Create a clock pulse, about 2us (freq 500Khz)
             delay_us(1);
             DS1302_SCLK_DATA = 1;
             delay_us(1);
             //-----------------------------------
             // Read bit when sclk is high
             //-----------------------------------
             dat = dat >> 1; // shift bit one step to left
             if(DS1302_IO_DATA) dat.F7 = 1; else dat.F7 = 0; // input data
         }
         DS1302_CLOCK_DATA[cc2] = dat;  // store result int array
     }
     //-------------------------------------------------------------------------
     // Whole operation completed, set IO line back to default
     //-------------------------------------------------------------------------
     DS1302_SCLK_DATA = 0;
     DS1302_IO_TRIS = 1;  // Change IO line to input
     DS1302_IO_DATA = 0;  // Change IO line to input
     DS1302_RST_DATA = 0; // Hold reset line, internal clock still running
}

/*******************************************************************************
*  input: -                                                                    *
********************************************************************************
*  output: -                                                                   *
********************************************************************************
*  comment: Write values from DS1302_CLOCK_DATA array to DS1302 registers      *
*******************************************************************************/
extern void DS1302_CLOCK_WRITE()
{
     tUI8 cc1,cc2,dat;     // cc1 and cc2 are used for counter

     DS1302_IO_TRIS = 0;  // Change IO line to output
     DS1302_RST_DATA = 1; // Release reset line and prepare for transfer

     dat = 0b10111110;    // CLOCK WRITE BURST COMMAND, <1><R/#C><1><1><1><1><1><R/#W>
     for(cc1=0;cc1<8;cc1++){
         if(dat.F0) DS1302_IO_DATA = 1; else DS1302_IO_DATA = 0; // Output data
         dat = dat >> 1;  // Shift bit one step to the right.

         DS1302_SCLK_DATA = 0;  // Create a clock pulse, about 2us (freq 500Khz)
         delay_us(1);
         DS1302_SCLK_DATA = 1;
         delay_us(1);
         DS1302_SCLK_DATA = 0;
     }
     //-------------------------------------------------------------------------
     // After sending command, next is to write all 8 bytes of data
     //-------------------------------------------------------------------------
     for(cc2=0;cc2<8;cc2++){
         dat = DS1302_CLOCK_DATA[cc2];
         for(cc1=0;cc1<8;cc1++){
             if(dat.F0) DS1302_IO_DATA = 1; else DS1302_IO_DATA = 0; // Output data
             dat = dat >> 1;  // Shift bit one step to the right.

             DS1302_SCLK_DATA = 0;  // Create a clock pulse, about 2us (freq 500Khz)
             delay_us(1);
             DS1302_SCLK_DATA = 1;
             delay_us(1);
             DS1302_SCLK_DATA = 0;
         }
     }
     //-------------------------------------------------------------------------
     // Whole operation completed, set IO line back to default
     //-------------------------------------------------------------------------
     DS1302_SCLK_DATA = 0;
     DS1302_IO_TRIS = 1;  // Change IO line to input
     DS1302_IO_DATA = 0;  // Change IO line to input
     DS1302_RST_DATA = 0; // Hold reset line, internal clock still running
}

/*******************************************************************************
*  input: -                                                                    *
********************************************************************************
*  output: -                                                                   *
********************************************************************************
*  comment: Read the values from DS1302 RAM to DS1302_RAM_DATA                 *
*******************************************************************************/
extern void DS1302_RAM_READ()
{
     tUI8 cc1,cc2,dat;     // cc1 and cc2 are used for counter

     DS1302_IO_TRIS = 0;  // Change IO line to output
     DS1302_RST_DATA = 1; // Release reset line and prepare for transfer

     dat = 0b11111111;    // CLOCK READ BURST COMMAND, <1><R/#C><1><1><1><1><1><R/#W>
     for(cc1=0;cc1<8;cc1++){
         if(dat.F0) DS1302_IO_DATA = 1; else DS1302_IO_DATA = 0; // Output data
         dat = dat >> 1;  // Shift bit one step to the right.

         DS1302_SCLK_DATA = 0;  // Create a clock pulse, about 2us (freq 500Khz)
         delay_us(1);
         DS1302_SCLK_DATA = 1;
         delay_us(1);
         DS1302_SCLK_DATA = 0;
     }
     //-------------------------------------------------------------------------
     // After sending command, next is to read all 31 bytes of data
     //-------------------------------------------------------------------------
     DS1302_IO_TRIS = 1;  // Change IO line to Input, to receive data

     for(cc2=0;cc2<31;cc2++){
         for(cc1=0;cc1<8;cc1++){
             DS1302_SCLK_DATA = 0;  // Create a clock pulse, about 2us (freq 500Khz)
             delay_us(1);
             DS1302_SCLK_DATA = 1;
             delay_us(1);
             //-----------------------------------
             // Read bit when sclk is high
             //-----------------------------------
             dat = dat >> 1; // shift bit one step to left
             if(DS1302_IO_DATA) dat.F7 = 1; else dat.F7 = 0; // input data

             }
         DS1302_RAM_DATA[cc2] = dat;  // store result int array
     }
     //-------------------------------------------------------------------------
     // Whole operation completed, set IO line back to default
     //-------------------------------------------------------------------------
     DS1302_SCLK_DATA = 0;
     DS1302_IO_TRIS = 1;  // Change IO line to input
     DS1302_IO_DATA = 0;  // Change IO line to input
     DS1302_RST_DATA = 0; // Hold reset line, internal clock still running
}

/*******************************************************************************
*  input: -                                                                    *
********************************************************************************
*  output: -                                                                   *
********************************************************************************
*  comment: Write the values from DS1302_RAM_DATA array to DS1302 RAM          *
*******************************************************************************/
extern void DS1302_RAM_WRITE()
{
     tUI8 cc1,cc2,dat;     // cc1 and cc2 are used for counter

     DS1302_IO_TRIS = 0;  // Change IO line to output
     DS1302_RST_DATA = 1; // Release reset line and prepare for transfer

     dat = 0b11111110;    // CLOCK WRITE BURST COMMAND, <1><R/#C><1><1><1><1><1><R/#W>
     for(cc1=0;cc1<8;cc1++){
         if(dat.F0) DS1302_IO_DATA = 1; else DS1302_IO_DATA = 0; // Output data
         dat = dat >> 1;  // Shift bit one step to the right.

         DS1302_SCLK_DATA = 0;  // Create a clock pulse, about 2us (freq 500Khz)
         delay_us(1);
         DS1302_SCLK_DATA = 1;
         delay_us(1);
         DS1302_SCLK_DATA = 0;
     }
     //-------------------------------------------------------------------------
     // After sending command, next is to write all 31 bytes of data
     //-------------------------------------------------------------------------
     for(cc2=0;cc2<31;cc2++){
         dat = DS1302_RAM_DATA[cc2];
         for(cc1=0;cc1<8;cc1++){
             if(dat.F0) DS1302_IO_DATA = 1; else DS1302_IO_DATA = 0; // Output data
             dat = dat >> 1;  // Shift bit one step to the right.

             DS1302_SCLK_DATA = 0;  // Create a clock pulse, about 2us (freq 500Khz)
             delay_us(1);
             DS1302_SCLK_DATA = 1;
             delay_us(1);
             DS1302_SCLK_DATA = 0;
         }
     }
     //-------------------------------------------------------------------------
     // Whole operation completed, set IO line back to default
     //-------------------------------------------------------------------------
     DS1302_SCLK_DATA = 0;
     DS1302_IO_TRIS = 1;  // Change IO line to input
     DS1302_IO_DATA = 0;  // Change IO line to input
     DS1302_RST_DATA = 0; // Hold reset line, internal clock still running
}
