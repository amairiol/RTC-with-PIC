//------------------------------------------------------------------------------
//                        Oliver Fádi Amairi
//            USE DS1302 RTC module with PIC16F877 microcontroller
//    To develope this code I used code generator for the timers and I used
//            an example code that I found in this link:
//          https://forum.mikroe.com/viewtopic.php?p=70949
//------------------------------------------------------------------------------

#define DS1302_RST_TRIS   TRISB.F7   // Modify these code to fix your IO lines
#define DS1302_IO_TRIS    TRISB.F6
#define DS1302_SCLK_TRIS   TRISB.F5

#define DS1302_RST_DATA    PORTB.F7 // Modify these code to fix your IO lines
#define DS1302_IO_DATA     PORTB.F6
#define DS1302_SCLK_DATA   PORTB.F5

#define BUTTON PORTB.F0


void DS1302_INIT();
void DS1302_CLOCK_READ();
void DS1302_CLOCK_WRITE();
void DS1302_RAM_READ();
void DS1302_RAM_WRITE();
void init_interrupts();
void init_timer0();
void init_timer1();
void interrupt();
unsigned char DS1302_format_to_min_or_sec(unsigned char x);
void write_segments(unsigned char time);

unsigned char second, minute, hour;
unsigned int scaler;
char segments;
unsigned char READ_ENABLE;
unsigned char command;
unsigned char DS1302_CLOCK_DATA[8];
unsigned char DS1302_RAM_DATA[31];


void main(){
    TRISC = 0x00;                                                                   //port C.F0...F4 --> enable segments F5...F7 --> DS1302 controll bits
    TRISB = 0x01;                                                                   //port B.F0 --> Push button
    TRISD = 0x00;                                                                   //port D = output number
    PORTB = 0x00;
    PORTD = 0x00;
    second = 0;
    minute = 0;
    hour = 0;
    scaler = 0;
    READ_ENABLE = 0;
    segments = 0;
    DS1302_INIT();
    init_timer0();
    init_timer1();
    init_interrupts();
    DS1302_CLOCK_DATA[0] = second;
    DS1302_CLOCK_DATA[1] = minute;
    DS1302_CLOCK_DATA[2] = hour;
    //at the first time set the time and upload the code
    //after you do this comment the following line and upload again
    //DS1302_CLOCK_WRITE();

    DS1302_CLOCK_READ();
    while(1)
    {
       if(READ_ENABLE){ DS1302_CLOCK_READ(); READ_ENABLE = 0; }

       second = DS1302_format_to_min_or_sec(DS1302_CLOCK_DATA[0]);
       minute = DS1302_format_to_min_or_sec(DS1302_CLOCK_DATA[1]);
       (BUTTON) ? write_segments(minute) : write_segments(second);

    }

}


void DS1302_INIT(){
     DS1302_RST_TRIS = 0; // Define this pin as output
     DS1302_IO_TRIS = 1;  // Define this pin as input for the start, need to change when Write
     DS1302_SCLK_TRIS = 0; // Define this pin as output

     DS1302_RST_DATA = 0;  // Hold DS1302 @ reset mode, int clock still running
     DS1302_SCLK_DATA = 0; // Sclk always begins at low
     DS1302_IO_DATA = 0;   // Hold IO data line low
}
//------------------------------------------------------------------------------
// Read all clock data sec, min, hour.......etc into array
//------------------------------------------------------------------------------
void DS1302_CLOCK_READ(){
     unsigned char cc1,cc2,dat;     // cc1 and cc2 are used for counter

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
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void DS1302_CLOCK_WRITE(){
     unsigned char cc1,cc2,dat;     // cc1 and cc2 are used for counter

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
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void DS1302_RAM_READ(){
     unsigned char cc1,cc2,dat;     // cc1 and cc2 are used for counter

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
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void DS1302_RAM_WRITE(){
     unsigned char cc1,cc2,dat;     // cc1 and cc2 are used for counter

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
void init_interrupts()
{
// Interrupt Registers
  INTCON = 0;           // clear the interrpt control register
  INTCON.TMR0IE = 1;        // bit5 TMR0 Overflow Interrupt Enable bit...1 = Enables the TMR0 interrupt
  PIR1.TMR1IF = 0;            // clear timer1 interupt flag TMR1IF
  PIE1.TMR1IE  =   1;         // enable Timer1 interrupts
  INTCON.TMR0IF = 0;        // bit2 clear timer 0 interrupt flag
  INTCON.GIE = 1;           // bit7 global interrupt enable
  INTCON.PEIE = 1;          // bit6 Peripheral Interrupt Enable bit...1 = Enables all unmasked peripheral interrupts
}

void init_timer0()
{
//Timer0 Registers Prescaler= 32 - TMR0 Preset = 6 - Freq = 500.00 Hz - Period = 0.002000 seconds
OPTION_REG.T0CS = 0;  // bit 5  TMR0 Clock Source Select bit...0 = Internal Clock (CLKO) 1 = Transition on T0CKI pin
OPTION_REG.T0SE = 0;  // bit 4 TMR0 Source Edge Select bit 0 = low/high 1 = high/low
OPTION_REG.PSA = 0;   // bit 3  Prescaler Assignment bit...0 = Prescaler is assigned to the Timer0
OPTION_REG.PS2 = 0;   // bits 2-0  PS2:PS0: Prescaler Rate Select bits
OPTION_REG.PS1 = 1;
OPTION_REG.PS0 = 1;
TMR0 = 6;             // preset for timer register
                                                                  // preset for timer register
}

void init_timer1()
{
    //Timer1 Registers Prescaler= 8 - TMR1 Preset = 3036 - Freq = 8.00 Hz - Period = 0.125000 seconds
T1CON.T1CKPS1 = 1;   // bits 5-4  Prescaler Rate Select bits
T1CON.T1CKPS0 = 1;   // bit 4
T1CON.T1OSCEN = 1;   // bit 3 Timer1 Oscillator Enable Control bit 1 = on
T1CON.T1SYNC = 1;    // bit 2 Timer1 External Clock Input Synchronization Control bit...1 = Do not synchronize external clock input
T1CON.TMR1CS = 0;    // bit 1 Timer1 Clock Source Select bit...0 = Internal clock (FOSC/4)
T1CON.TMR1ON = 1;    // bit 0 enables timer
TMR1H = 11;             // preset for timer1 MSB register
TMR1L = 220;             // preset for timer1 LSB register

}
void interrupt()
{
  if (INTCON.TMR0IF ==1) // timer 0 interrupt flag
  {
    (segments==4)? segments = 0 : segments++;
    INTCON.TMR0IF = 0;         // clear the flag
    INTCON.TMR0IE = 1;         // reenable the interrupt
    TMR0 = 6;           // reset the timer preset count
  }

    // Timer1 Interrupt - Freq = 8.00 Hz - Period = 0.125000 seconds
  if (PIR1.TMR1IF == 1) // timer 1 interrupt flag
  {
    (scaler==7) ? scaler = 0 : scaler++;
    if(scaler==0) READ_ENABLE = 1;
    PIR1.TMR1IF = 0;           // interrupt must be cleared by software
    PIE1.TMR1IE  =   1;        // reenable the interrupt
    TMR1H = 11;             // preset for timer1 MSB register
    TMR1L = 220;             // preset for timer1 LSB register
  }

}

 unsigned char DS1302_format_to_min_or_sec(unsigned char x){
     unsigned char ten,ind, ret;
     ten = x.F6*4+x.F5*2+x.F4;
     ind = x.F3*8+x.F2*4+x.F1*2+x.F0;
     ret = ten*10 + ind;
     return ret;
}

void write_segments(unsigned char time){
      switch(segments)
       {
           case 0:
           switch(time%10)                                                        //decode the number
            {
              case 0: PORTD = 0xC0; break;
              case 1: PORTD = 0xF9; break;
              case 2: PORTD = 0xA4; break;
              case 3: PORTD = 0xB0; break;
              case 4: PORTD = 0x99; break;
              case 5: PORTD = 0x92; break;
              case 6: PORTD = 0x82; break;
              case 7: PORTD = 0xF8; break;
              case 8: PORTD = 0x80; break;
              case 9: PORTD = 0x90; break;
            }
            PORTC = 0x80;                                                           //enable first segment
            break;

           case 1:
             switch(time/10)                                                      //decode the number
              {
                case 0: PORTD = 0xC0; break;
                case 1: PORTD = 0xF9; break;
                case 2: PORTD = 0xA4; break;
                case 3: PORTD = 0xB0; break;
                case 4: PORTD = 0x99; break;
                case 5: PORTD = 0x92; break;
                case 6: PORTD = 0x82; break;
                case 7: PORTD = 0xF8; break;
                case 8: PORTD = 0x80; break;
                case 9: PORTD = 0x90; break;
              }
             PORTC = 0x40;                                                           //enable second segment
                break;

           case 2: PORTC = 0x20; break;                                             //enable third segment
           case 3: PORTC = 0x10; break;                                             //enable fourth segment
       }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
