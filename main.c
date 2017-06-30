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

#define DS1302_RST_DATA    PORTB.F7
#define DS1302_IO_DATA     PORTB.F6
#define DS1302_SCLK_DATA   PORTB.F5

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
#define X5 PORTA.F2
#define X6 PORTA.F1

#define HC4017_CLK PORTC.F2
#define HC4017_RESET PORTC.F3

//define inputs
#define BUTTON PORTB.F0

void DS1302_INIT();
void DS1302_CLOCK_READ();
void DS1302_CLOCK_WRITE();
void DS1302_RAM_READ();
void DS1302_RAM_WRITE();
void HC4017_INIT();
void init_interrupts();
void init_timer0();
void init_timer1();
void interrupt();
unsigned char DS1302_format_to_min_or_sec(unsigned char x);
unsigned char min_or_sec_to_DS1302_format(unsigned char y);
unsigned char hour_to_DS1302_format(unsigned char z);
unsigned char DS1302_format_to_hour(unsigned char v);
void write_time();
void write_sec();
void init_time (unsigned char sec, unsigned char min, unsigned char hour, unsigned char CH_FLAG);

unsigned char second, minute, hour;
unsigned int scaler;
char segments;
char second_segments;
unsigned char READ_ENABLE;
unsigned char command;
unsigned char DS1302_CLOCK_DATA[8];
unsigned char DS1302_RAM_DATA[31];


void main(){
    TRISA = 0x00;
    TRISB = 0x01;
    TRISC = 0x00;
    TRISD = 0x00;
   
    PORTA = 0x00;                                                                   //port D = output number
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
   
    second = 0;
    minute = 0;
    hour = 0;
    scaler = 0;
    READ_ENABLE = 0;
    segments = 0;
    second_segments = 0;
    DS1302_INIT();
    HC4017_INIT();
    init_timer0();
    init_timer1();
    init_interrupts();
    //at the first time set the time and upload the code
    //after you do this comment the following line and upload again
    //init_time(00,47,20,0);                                                            //parameters: (second, minute, hour, CH_FLAG)
    DS1302_CLOCK_READ();
    while(1)
    {
       if(READ_ENABLE){ DS1302_CLOCK_READ(); READ_ENABLE = 0; }
       second = DS1302_format_to_min_or_sec(DS1302_CLOCK_DATA[0]);
       minute = DS1302_format_to_min_or_sec(DS1302_CLOCK_DATA[1]);
       hour = DS1302_format_to_hour(DS1302_CLOCK_DATA[2]);
       write_sec();
       write_time();
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

void HC4017_INIT()
{
   HC4017_CLK = 0;
   delay_us(1);
   HC4017_RESET = 1;
   delay_us(1);
   HC4017_RESET = 0;
   delay_us(1);
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
    HC4017_CLK = ~HC4017_CLK;
    if(HC4017_CLK)
       if(second_segments != 9)
          second_segments++;
       else
          second_segments = 0;
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

unsigned char DS1302_format_to_min_or_sec(unsigned char x)
{
     unsigned char ten,ind, ret;
     ten = x.F6*4+x.F5*2+x.F4;
     ind = x.F3*8+x.F2*4+x.F1*2+x.F0;
     ret = ten*10 + ind;
     return ret;
}

unsigned char min_or_sec_to_DS1302_format(unsigned char y)
{
     unsigned char ret;
     switch(y/10)
     {
         case 0: ret.F6 = 0; ret.F5 = 0; ret.F4 = 0; break;
         case 1: ret.F6 = 0; ret.F5 = 0; ret.F4 = 1; break;
         case 2: ret.F6 = 0; ret.F5 = 1; ret.F4 = 0; break;
         case 3: ret.F6 = 0; ret.F5 = 1; ret.F4 = 1; break;
         case 4: ret.F6 = 1; ret.F5 = 0; ret.F4 = 0; break;
         case 5: ret.F6 = 1; ret.F5 = 0; ret.F4 = 1; break;
         default : ret.F6 = 0; ret.F5 = 0; ret.F4 = 0; break;
     }
     switch(y%10)
     {
         case 0: ret.F3 = 0; ret.F2 = 0; ret.F1 = 0; ret.F0 = 0; break;
         case 1: ret.F3 = 0; ret.F2 = 0; ret.F1 = 0; ret.F0 = 1; break;
         case 2: ret.F3 = 0; ret.F2 = 0; ret.F1 = 1; ret.F0 = 0; break;
         case 3: ret.F3 = 0; ret.F2 = 0; ret.F1 = 1; ret.F0 = 1; break;
         case 4: ret.F3 = 0; ret.F2 = 1; ret.F1 = 0; ret.F0 = 0; break;
         case 5: ret.F3 = 0; ret.F2 = 1; ret.F1 = 0; ret.F0 = 1; break;
         case 6: ret.F3 = 0; ret.F2 = 1; ret.F1 = 1; ret.F0 = 0; break;
         case 7: ret.F3 = 0; ret.F2 = 1; ret.F1 = 1; ret.F0 = 1; break;
         case 8: ret.F3 = 1; ret.F2 = 0; ret.F1 = 0; ret.F0 = 0; break;
         case 9: ret.F3 = 1; ret.F2 = 0; ret.F1 = 0; ret.F0 = 1; break;
         default: ret.F3 = 0; ret.F2 = 0; ret.F1 = 0; ret.F0 = 0; break;
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
            //enable first segment
            DIGIT1 = 1;
            DIGIT2 = 0;
            DIGIT3 = 0;
            DIGIT4 = 0;
            break;

           case 1:
             switch(minute/10)                                                      //decode the number
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
             //enable second segment
            DIGIT1 = 0;
            DIGIT2 = 1;
            DIGIT3 = 0;
            DIGIT4 = 0;
                break;

           case 2:
             switch(hour%10)                                                      //decode the number
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

            //enable third segment
            DIGIT1 = 0;
            DIGIT2 = 0;
            DIGIT3 = 1;
            DIGIT4 = 0;
            break;
           case 3:
             switch(hour/10)                                                      //decode the number
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
              //enable fourth segment
            DIGIT1 = 0;
            DIGIT2 = 0;
            DIGIT3 = 0;
            DIGIT4 = 1;
              break;
       }
}

unsigned char hour_to_DS1302_format(unsigned char z)
{
     unsigned char ret;
     ret.F7 = 0;                     //set 24 hour format
     ret.F6 = 0;

     switch(z/10)
     {
        case 0: ret.F5 = 0; ret.F4 = 0; break;
        case 1: ret.F5 = 0; ret.F4 = 1; break;
        case 2: ret.F5 = 1; ret.F4 = 0; break;
     }
     switch(z%10)
     {
         case 0: ret.F3 = 0; ret.F2 = 0; ret.F1 = 0; ret.F0 = 0; break;
         case 1: ret.F3 = 0; ret.F2 = 0; ret.F1 = 0; ret.F0 = 1; break;
         case 2: ret.F3 = 0; ret.F2 = 0; ret.F1 = 1; ret.F0 = 0; break;
         case 3: ret.F3 = 0; ret.F2 = 0; ret.F1 = 1; ret.F0 = 1; break;
         case 4: ret.F3 = 0; ret.F2 = 1; ret.F1 = 0; ret.F0 = 0; break;
         case 5: ret.F3 = 0; ret.F2 = 1; ret.F1 = 0; ret.F0 = 1; break;
         case 6: ret.F3 = 0; ret.F2 = 1; ret.F1 = 1; ret.F0 = 0; break;
         case 7: ret.F3 = 0; ret.F2 = 1; ret.F1 = 1; ret.F0 = 1; break;
         case 8: ret.F3 = 1; ret.F2 = 0; ret.F1 = 0; ret.F0 = 0; break;
         case 9: ret.F3 = 1; ret.F2 = 0; ret.F1 = 0; ret.F0 = 1; break;
         default: ret.F3 = 0; ret.F2 = 0; ret.F1 = 0; ret.F0 = 0; break;
     }
     return ret;
}

unsigned char DS1302_format_to_hour(unsigned char v)
{
     unsigned char ret, ten, ind;
     ten = v.F5*2 + v.F4;
     ind = v.F3*8 + v.F2*4 + v.F1*2 + v.F0;
     ret = ten*10+ind;
     return ret;
}

void write_sec()
{
    unsigned char s;
    s = second%6;
    switch(s)
    {
       case 0: X1 = 1; X2 = 0; X3 = 0; X4 = 0; X5 = 0; X6 = 0; break;
       case 1: X1 = 1; X2 = 1; X3 = 0; X4 = 0; X5 = 0; X6 = 0; break;
       case 2: X1 = 1; X2 = 1; X3 = 1; X4 = 0; X5 = 0; X6 = 0; break;
       case 3: X1 = 1; X2 = 1; X3 = 1; X4 = 1; X5 = 0; X6 = 0; break;
       case 4: X1 = 1; X2 = 1; X3 = 1; X4 = 1; X5 = 1; X6 = 0; break;
       case 5: X1 = 1; X2 = 1; X3 = 1; X4 = 1; X5 = 1; X6 = 1; break;
       default: X1 = 0; X2 = 1; X3 = 0; X4 = 1; X5 = 0; X6 = 1; break;
    }
    
}
void init_time (unsigned char sec, unsigned char min, unsigned char hour, unsigned char CH_FLAG)
{
    DS1302_CLOCK_DATA[0] = min_or_sec_to_DS1302_format(sec);
    DS1302_CLOCK_DATA[1] = min_or_sec_to_DS1302_format(min);
    DS1302_CLOCK_DATA[2] = hour_to_DS1302_format(hour);
    DS1302_CLOCK_DATA[0].F7 = CH_FLAG;
    DS1302_CLOCK_WRITE();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
