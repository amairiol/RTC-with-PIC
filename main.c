/*******************************************************************************
*                        Oliver Fádi Amairi                                    *
*            USE DS1302 RTC module with PIC16F877A microcontroller              *
*    To develope this code I used code generator for the timers and I used     *
*            an example code that I found in this link:                        *
*          https://forum.mikroe.com/viewtopic.php?p=70949                      *
*******************************************************************************/
//   {    |    &

/*******************************************************************************
*                                  Includes                                    *
*******************************************************************************/
#include "DS1302.c"
#include "datatypes.h"

/*******************************************************************************
*                                  Defines                                     *
*******************************************************************************/

//define segment's IO ports
#define DIGIT1 PORTC.F7
#define DIGIT2 PORTC.F6
#define DIGIT3 PORTC.F5
#define DIGIT4 PORTC.F4


//define matrix IO lines
#define X1 PORTB.F1
#define X2 PORTB.F2
#define X3 PORTB.F3
#define X4 PORTB.F4
#define X5 PORTA.F1
#define X6 PORTA.F3

#define HC4017_CLK PORTA.F0
#define HC4017_RESET PORTA.F2

//define inputs
#define BUTTON PORTB.F0

void DIO_Config();
void HC4017_INIT();
void init_interrupts();
void init_timer0();
void init_timer1();
void interrupt();
tUI8 DS1302_format_to_min_or_sec(tUI8 num);
tUI8 min_or_sec_to_DS1302_format(tUI8 num);
tUI8 hour_to_DS1302_format(tUI8 num);
tUI8 DS1302_format_to_hour(tUI8 num);
void write_time();
void init_time (tUI8 sec, tUI8 min, tUI8 hour, tBOOL CH_FLAG);

tUI8 second, minute, hour;
tUI8 scaler;
tUI8 segments;
tUI8 second_segments;
tBOOL READ_ENABLE;
tUI8 command;

enum eNumbers{
     zero = 0xC0,
     one = 0xF9,
     two = 0xA4,
     three = 0xB0,
     four = 0x99,
     five = 0x92,
     six = 0x82,
     seven = 0xF8,
     eight = 0x80,
     nine = 0x90
};

enum eTimerConsts{
     Timer0Preset = 6,
     Timer1High = 11,
     Timer1Low = 220
};


void main(){

    DIO_Config();
    
    X1 = TRUE;
    X2 = TRUE;
    X3 = TRUE;
    X4 = TRUE;
    X5 = TRUE;
    X6 = TRUE;

    HC4017_CLK = TRUE;
    HC4017_RESET = TRUE;
    
    //at first it does not matter what is the values of these variables, so I init them to 0
    second = 0;
    minute = 0;
    hour = 0;
    scaler = 0;
    READ_ENABLE = FALSE;
    segments = 0;
    second_segments = 0;
    DS1302_INIT();
    //HC4017_INIT();
    init_timer0();
    init_timer1();
    init_interrupts();
    //at the first time set the time and upload the code
    //after you do this comment the following line and upload again
    //init_time(00,31,17,0);
    DS1302_CLOCK_READ();
    while(TRUE)
    {
       if(READ_ENABLE){ DS1302_CLOCK_READ(); READ_ENABLE = FALSE; }
       second = DS1302_format_to_min_or_sec(DS1302_CLOCK_DATA[SecondRegister]);
       minute = DS1302_format_to_min_or_sec(DS1302_CLOCK_DATA[MinuteRegister]);
       hour = DS1302_format_to_hour(DS1302_CLOCK_DATA[HourRegister]);
       write_time();
    }
}

void DIO_Config()
{
    ADCON1 = 7;

    TRISA = 0x00;
    TRISB = 0x01;
    TRISC = 0x00;
    TRISD = 0x00;
    TRISE = 0x00;

    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
    PORTE = 0x00;


}


void HC4017_INIT()
{
   HC4017_CLK = FALSE;
   delay_us(1);
   HC4017_RESET = TRUE;
   delay_us(1);
   HC4017_RESET = FALSE;
   delay_us(1);
}
void init_interrupts()
{
// Interrupt Registers
  INTCON = FALSE;           // clear the interrpt control register
  INTCON.TMR0IE = TRUE;        // bit5 TMR0 Overflow Interrupt Enable bit...1 = Enables the TMR0 interrupt
  PIR1.TMR1IF = FALSE;            // clear timer1 interupt flag TMR1IF
  PIE1.TMR1IE  = TRUE;         // enable Timer1 interrupts
  INTCON.TMR0IF = FALSE;        // bit2 clear timer 0 interrupt flag
  INTCON.GIE = TRUE;           // bit7 global interrupt enable
  INTCON.PEIE = TRUE;          // bit6 Peripheral Interrupt Enable bit...1 = Enables all unmasked peripheral interrupts
}

void init_timer0()
{
                                  //Timer0 Registers Prescaler= 32 - TMR0 Preset = 6 - Freq = 500.00 Hz - Period = 0.002000 seconds
OPTION_REG.T0CS = FALSE;          // bit 5  TMR0 Clock Source Select bit...0 = Internal Clock (CLKO) 1 = Transition on T0CKI pin
OPTION_REG.T0SE = FALSE;          // bit 4 TMR0 Source Edge Select bit 0 = low/high 1 = high/low
OPTION_REG.PSA = FALSE;           // bit 3  Prescaler Assignment bit...0 = Prescaler is assigned to the Timer0
OPTION_REG.PS2 = FALSE;           // bits 2-0  PS2:PS0: Prescaler Rate Select bits
OPTION_REG.PS1 = TRUE;
OPTION_REG.PS0 = TRUE;
TMR0 = Timer0Preset;             // preset for timer register
}

void init_timer1()
{
                                //Timer1 Registers Prescaler= 8 - TMR1 Preset = 3036 - Freq = 8.00 Hz - Period = 0.125000 seconds
T1CON.T1CKPS1 = TRUE;           // bits 5-4  Prescaler Rate Select bits
T1CON.T1CKPS0 = TRUE;           // bit 4
T1CON.T1OSCEN = TRUE;           // bit 3 Timer1 Oscillator Enable Control bit 1 = on
T1CON.T1SYNC = TRUE;            // bit 2 Timer1 External Clock Input Synchronization Control bit...1 = Do not synchronize external clock input
T1CON.TMR1CS = FALSE;           // bit 1 Timer1 Clock Source Select bit...0 = Internal clock (FOSC/4)
T1CON.TMR1ON = TRUE;            // bit 0 enables timer
TMR1H = Timer1High;             // preset for timer1 MSB register
TMR1L = Timer1Low;              // preset for timer1 LSB register

}

void interrupt()
{
  if (INTCON.TMR0IF == TRUE) // timer 0 interrupt flag
  {
    (segments==4)? segments = 0 : segments++;
    INTCON.TMR0IF = FALSE;                  // clear the flag
    INTCON.TMR0IE = TRUE;                  // reenable the interrupt
    TMR0 = Timer0Preset;                // reset the timer preset count
  }

                                        // Timer1 Interrupt - Freq = 8.00 Hz - Period = 0.125000 seconds
  if (PIR1.TMR1IF == TRUE)              // timer 1 interrupt flag
  {
    (scaler==7) ? scaler = 0 : scaler++;
    if(scaler==0) 
    {
     READ_ENABLE = TRUE;
     X1 = ~X1;
     X2 = ~X2;
     X3 = ~X3;
     X4 = ~X4;
     X5 = ~X5;
     X6 = ~X6;
     HC4017_CLK = ~HC4017_CLK;
     HC4017_RESET = ~HC4017_RESET;
    }
    PIR1.TMR1IF = FALSE;                // interrupt must be cleared by software
    PIE1.TMR1IE  = TRUE;              // reenable the interrupt
    TMR1H = Timer1High;                 // preset for timer1 MSB register
    TMR1L = Timer1Low;                  // preset for timer1 LSB register
  }

}

unsigned char DS1302_format_to_min_or_sec(unsigned char num)
{
     tUI8 ten,ind, ret;
     ten = num.F6*4+num.F5*2+num.F4;
     ind = num.F3*8+num.F2*4+num.F1*2+num.F0;
     ret = ten*10 + ind;
     return ret;
}

unsigned char min_or_sec_to_DS1302_format(unsigned char num)
{
     tUI8 ret;
     switch(num/10)
     {
         case 0: ret.F6 = FALSE; ret.F5 = FALSE; ret.F4 = FALSE; break;
         case 1: ret.F6 = FALSE; ret.F5 = FALSE; ret.F4 = TRUE; break;
         case 2: ret.F6 = FALSE; ret.F5 = TRUE; ret.F4 = FALSE; break;
         case 3: ret.F6 = FALSE; ret.F5 = TRUE; ret.F4 = TRUE; break;
         case 4: ret.F6 = TRUE; ret.F5 = FALSE; ret.F4 = FALSE; break;
         case 5: ret.F6 = TRUE; ret.F5 = FALSE; ret.F4 = TRUE; break;
         default : ret.F6 = FALSE; ret.F5 = FALSE; ret.F4 = FALSE; break;
     }
     switch(num%10)
     {
         case 0: ret.F3 = FALSE; ret.F2 = FALSE; ret.F1 = FALSE; ret.F0 = FALSE; break;
         case 1: ret.F3 = FALSE; ret.F2 = FALSE; ret.F1 = FALSE; ret.F0 = TRUE; break;
         case 2: ret.F3 = FALSE; ret.F2 = FALSE; ret.F1 = TRUE; ret.F0 = FALSE; break;
         case 3: ret.F3 = FALSE; ret.F2 = FALSE; ret.F1 = TRUE; ret.F0 = TRUE; break;
         case 4: ret.F3 = FALSE; ret.F2 = TRUE; ret.F1 = FALSE; ret.F0 = FALSE; break;
         case 5: ret.F3 = FALSE; ret.F2 = TRUE; ret.F1 = FALSE; ret.F0 = TRUE; break;
         case 6: ret.F3 = FALSE; ret.F2 = TRUE; ret.F1 = TRUE; ret.F0 = FALSE; break;
         case 7: ret.F3 = FALSE; ret.F2 = TRUE; ret.F1 = TRUE; ret.F0 = TRUE; break;
         case 8: ret.F3 = TRUE; ret.F2 = FALSE; ret.F1 = FALSE; ret.F0 = FALSE; break;
         case 9: ret.F3 = TRUE; ret.F2 = FALSE; ret.F1 = FALSE; ret.F0 = TRUE; break;
         default: ret.F3 = FALSE; ret.F2 = FALSE; ret.F1 = FALSE; ret.F0 = FALSE; break;
     }
     return ret;

}

void write_time()
{
      switch(segments)
       {
           case 0:
           switch(minute%10)                                                        //decode the number
            {
              case 0: PORTD = zero; break;
              case 1: PORTD = one; break;
              case 2: PORTD = two; break;
              case 3: PORTD = three; break;
              case 4: PORTD = four; break;
              case 5: PORTD = five; break;
              case 6: PORTD = six; break;
              case 7: PORTD = seven; break;
              case 8: PORTD = eight; break;
              case 9: PORTD = nine; break;
            }
            //enable first segment
            DIGIT1 = TRUE;
            DIGIT2 = FALSE;
            DIGIT3 = FALSE;
            DIGIT4 = FALSE;
            break;

           case 1:
             switch(minute/10)                                                      //decode the number
              {
                case 0: PORTD = zero; break;
                case 1: PORTD = one; break;
                case 2: PORTD = two; break;
                case 3: PORTD = three; break;
                case 4: PORTD = four; break;
                case 5: PORTD = five; break;
                case 6: PORTD = six; break;
                case 7: PORTD = seven; break;
                case 8: PORTD = eight; break;
                case 9: PORTD = nine; break;
              }
             //enable second segment
            DIGIT1 = FALSE;
            DIGIT2 = TRUE;
            DIGIT3 = FALSE;
            DIGIT4 = FALSE;
                break;

           case 2:
             switch(hour%10)                                                      //decode the number
              {
                case 0: PORTD = zero; break;
                case 1: PORTD = one; break;
                case 2: PORTD = two; break;
                case 3: PORTD = three; break;
                case 4: PORTD = four; break;
                case 5: PORTD = five; break;
                case 6: PORTD = six; break;
                case 7: PORTD = seven; break;
                case 8: PORTD = eight; break;
                case 9: PORTD = nine; break;
              }

            //enable third segment
            DIGIT1 = FALSE;
            DIGIT2 = FALSE;
            DIGIT3 = TRUE;
            DIGIT4 = FALSE;
            break;
           case 3:
             switch(hour/10)                                                      //decode the number
              {
                case 0: PORTD = zero; break;
                case 1: PORTD = one; break;
                case 2: PORTD = two; break;
                case 3: PORTD = three; break;
                case 4: PORTD = four; break;
                case 5: PORTD = five; break;
                case 6: PORTD = six; break;
                case 7: PORTD = seven; break;
                case 8: PORTD = eight; break;
                case 9: PORTD = nine; break;
              }
              //enable fourth segment
            DIGIT1 = FALSE;
            DIGIT2 = FALSE;
            DIGIT3 = FALSE;
            DIGIT4 = TRUE;
            break;
       }
}

unsigned char hour_to_DS1302_format(unsigned char num)
{
     tUI8 ret;
     ret.F7 = FALSE;                     //set 24 hour format
     ret.F6 = FALSE;

     switch(num/10)
     {
        case 0: ret.F5 = FALSE; ret.F4 = FALSE; break;
        case 1: ret.F5 = FALSE; ret.F4 = TRUE; break;
        case 2: ret.F5 = TRUE; ret.F4 = FALSE; break;
     }
     switch(num%10)
     {
         case 0: ret.F3 = FALSE; ret.F2 = FALSE; ret.F1 = FALSE; ret.F0 = FALSE; break;
         case 1: ret.F3 = FALSE; ret.F2 = FALSE; ret.F1 = FALSE; ret.F0 = TRUE; break;
         case 2: ret.F3 = FALSE; ret.F2 = FALSE; ret.F1 = TRUE; ret.F0 = FALSE; break;
         case 3: ret.F3 = FALSE; ret.F2 = FALSE; ret.F1 = TRUE; ret.F0 = TRUE; break;
         case 4: ret.F3 = FALSE; ret.F2 = TRUE; ret.F1 = FALSE; ret.F0 = FALSE; break;
         case 5: ret.F3 = FALSE; ret.F2 = TRUE; ret.F1 = FALSE; ret.F0 = TRUE; break;
         case 6: ret.F3 = FALSE; ret.F2 = TRUE; ret.F1 = TRUE; ret.F0 = FALSE; break;
         case 7: ret.F3 = FALSE; ret.F2 = TRUE; ret.F1 = TRUE; ret.F0 = TRUE; break;
         case 8: ret.F3 = TRUE; ret.F2 = FALSE; ret.F1 = FALSE; ret.F0 = FALSE; break;
         case 9: ret.F3 = TRUE; ret.F2 = FALSE; ret.F1 = FALSE; ret.F0 = TRUE; break;
         default: ret.F3 = FALSE; ret.F2 = FALSE; ret.F1 = FALSE; ret.F0 = FALSE; break;
     }
     return ret;
}

unsigned char DS1302_format_to_hour(unsigned char num)
{
     unsigned char ret, ten, ind;
     ten = num.F5*2 + num.F4;
     ind = num.F3*8 + num.F2*4 + num.F1*2 + num.F0;
     ret = ten*10+ind;
     return ret;
}

void init_time (tUI8 sec, tUI8 min, tUI8 hour, tBOOL CH_FLAG)
{
    DS1302_CLOCK_DATA[SecondRegister] = min_or_sec_to_DS1302_format(sec);
    DS1302_CLOCK_DATA[MinuteRegister] = min_or_sec_to_DS1302_format(min);
    DS1302_CLOCK_DATA[HourRegister] = hour_to_DS1302_format(hour);
    DS1302_CLOCK_DATA[SecondRegister].F7 = CH_FLAG;                           //The last bit in the second register is the Clock Halt Flag
    DS1302_CLOCK_WRITE();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
