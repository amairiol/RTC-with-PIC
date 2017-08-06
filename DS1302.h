/*******************************************************************************
*                        Oliver Fádi Amairi                                    *
*            USE DS1302 RTC module with PIC16F877A microcontroller              *
*    To develope this code I used code generator for the timers and I used     *
*            an example code that I found in this link:                        *
*          https://forum.mikroe.com/viewtopic.php?p=70949                      *
*******************************************************************************/

#ifndef DS1302
#define DS1302

/*******************************************************************************
*                                  Includes                                    *
*******************************************************************************/
#include "datatypes.h"


/*******************************************************************************
*                                  Defines                                    *
*******************************************************************************/

#define DS1302_RST_TRIS   TRISB.F7   // Modify these code to fix your IO lines
#define DS1302_IO_TRIS    TRISB.F6
#define DS1302_SCLK_TRIS   TRISB.F5

#define DS1302_RST_DATA    PORTB.F7
#define DS1302_IO_DATA     PORTB.F6
#define DS1302_SCLK_DATA   PORTB.F5


/*******************************************************************************
*                     Variable and function definitions                        *
*******************************************************************************/

tUI8 DS1302_CLOCK_DATA[8];
tUI8 DS1302_RAM_DATA[31];


void DS1302_INIT();
void DS1302_CLOCK_READ();
void DS1302_CLOCK_WRITE();
void DS1302_RAM_READ();
void DS1302_RAM_WRITE();

#endif
