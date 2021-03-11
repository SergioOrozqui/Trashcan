/* August 22, 2019
 * FILE: Lab5_PWM
 * Start by copying code from Lecture8_PWM.
 * The following is the code for the first part of Lab5.
 * Changing "Value" will change the number of iterations PWM output stays constant.
 * For larger Value, RC filter settles to constant voltage.
 */

#include "config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
struct item
{
    float weight;
    char* type;
    float volume;

};

#define SERVO_NEUTRAL 94
#define SERVO_FORWARD 38
#define SERVO_BACK 150  
#define STEP_FULL_TURN 200
#define STEP_QUART 50
#define STEP_THREEQ 150
#define STEP_TIME_DELAY 5//seconds
#define RTS _RF13 // Output, For potential hardware handshaking.
#define CTS _RF12 // Input, For potential hardware handshaking.
#define STEP_DUTY 312

char * message = {"abcde"};
int steps =0;
int STEP_MOVE = 0;
int SERVO_MOVE = 0;
char* types = { "Plastic","Glass","Metal","Trash" };
int dSec = 0, Sec = 0, Min = 0, Hr = 0;

void InitADC(void);
void I2Cinit(int BRG);
void I2CStart(void);
void I2CStop(void);
char I2Cgetbyte(void);
void I2Csendbyte(char data);
void us_delay(int i);
void InitServo(void);
void _ISRFAST _T3Interrupt(void);
void ADCStart();
void StepPlastic();
int StepCheckTime(int );
void StepGlass();
void StepTrash();
void StepMetal();
void InitTime();
void _ISR _T4Interrupt(void);
void MoveStep90();
void MoveStep270();
void MoveStep180();
void InitStepper();
void _ISRFAST _T2Interrupt();
void ms_delay(int);
void InitU2();
char PutU2(char c);
char GetU2();

void UpdateLCD()
{
    LCD_PutChar ( 'C' ) ;
    LCD_PutChar ( 'u' ) ;
    LCD_PutChar ( 'r' ) ;
    LCD_PutChar ( 'r' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 't' ) ;
    LCD_PutChar ( 'i' ) ;
    LCD_PutChar ( 'm' ) ;
    LCD_PutChar ( 'e' ) ;
    LCD_PutChar ( ':' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( 'm' ) ;
    LCD_PutChar ( 'e' ) ;
    LCD_PutChar ( '!' ) ;
    LCD_PutChar ( 'N' ) ;
    LCD_PutChar ( 'i' ) ;
    LCD_PutChar ( 'n' ) ;
    LCD_PutChar ( 'e' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 't' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 'y' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 'd' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( 'm' ) ;
    LCD_PutChar ( 'e' ) ;
    LCD_PutChar ( '!' ) ;
}
int ReadADC( int ch)
{
    AD1CHS = ch; // 1. select analog input channel
    // start sampling, automatic conversion will follow
    AD1CON1bits.SAMP = 1; // 2. Start sampling.
    while( !AD1CON1bits.DONE); //5. wait for conversion to complete
    AD1CON1bits.DONE = 0; // 6. clear flag. We are responsible see text.
    return ADC1BUF0; // 7. read the conversion results
} // ReadADC
int main(void)
{
    int c;
    bool updateLCD = false;
    TRISD = 0xFFFF;
    InitTime();
    InitServo();
    InitStepper();
    InitU2();
    LCD_Initialize();
    LCD_ClearScreen();
    LCD_PutChar ( 'N' ) ;
    LCD_PutChar ( 'i' ) ;
    LCD_PutChar ( 'n' ) ;
    LCD_PutChar ( 'e' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 't' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 'y' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 'd' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( 'm' ) ;
    LCD_PutChar ( 'e' ) ;
    LCD_PutChar ( '!' ) ;
    LCD_PutChar ( 'N' ) ;
    LCD_PutChar ( 'i' ) ;
    LCD_PutChar ( 'n' ) ;
    LCD_PutChar ( 'e' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 't' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 'y' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 'd' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( 'm' ) ;
    LCD_PutChar ( 'e' ) ;
    LCD_PutChar ( '!' ) ;
   
    //    I2Cinit(0x9D); //enable I2C
    //    I2CStart(); // initiate start condition
    //    I2Csendbyte(0x64); //begin communication with slave
    //    I2CStop(); // halt condition enable bit

    while (1)
    {
        if (PORTDbits.RD6 == 0)
        {
            updateLCD = true;
            //SERVO_MOVE = 1;
            
        }
        else if(PORTDbits.RD7 == 0)
        {
            //MoveStepQuart();
            LCD_ClearScreen();
            
        }
        else if(updateLCD)
        {
            UpdateLCD();
            updateLCD = false;
        }
        else
        {
            SERVO_MOVE = 0;
            STEP_MOVE = 0;
        }

//        for(c = 0; c<5;c++)
//        {
//            PutU2(message[c]);
//        }
//       
//        ms_delay(10);

    }
    return 0;
}//main

char PutU2(char c) {
    while (CTS); //wait for !CTS (active low)
    while (U2STAbits.UTXBF); // Wait if transmit buffer full.
    U2TXREG = c; // Write value to transmit FIFO
    return c;
}//putU2

char GetU2(void) {
    RTS = 0; // telling the other side !RTS
    while (!U2STAbits.URXDA); // wait
    RTS = 1; // telling the other side RTS
    return U2RXREG; // from receiving buffer
} //getU2

void _ISRFAST _T2Interrupt(void)
{   

    if(STEP_MOVE)
    {
        steps++;
        OC2RS = STEP_DUTY;
    }
    else 
    {
        OC2RS = 0;
    }
    _T2IF = 0;      // clear interrupt flag and exit
} // T4 Interrupt

void ms_delay(int N)
{
    us_delay(1000 * N);
}//ms_delay

void InitU2(void) {
    U2BRG = 34; // PIC24FJ128GA010 data sheet, 17.1 for calculation, Fcy= 16MHz.
    U2MODE = 0x8008; // See data sheet, pg148. Enable UART2, BRGH = 1,
    // Idle state = 1, 8 data, No parity, 1 Stop bit
    U2STA = 0x0400; // See data sheet, pg. 150, Transmit Enable
    // Following lines pertain Hardware handshaking
    TRISFbits.TRISF13 = 1; // enable RTS , output
    RTS = 1; // default status , not ready to send
}//InitU2

void MoveStep90(void)
{
    STEP_MOVE = 1;
    while(steps<STEP_QUART);
    STEP_MOVE = 0;
    steps = 0;
}//MoveStepQuart

void MoveStep270(void)
{
    MoveStep90();
    MoveStep90();
    MoveStep90();
}

void MoveStep180(void)
{
    MoveStep90();
    MoveStep90();
}

void InitStepper()
{
    T2CON = 0x8030; // enable TMR3, 1:256, 16 bit Timer, intclock
    PR2 = 625 - 1;  // 20 kHz for Lab5. see Equ. 14-1 of data sheet, Fcy=16MHz
    _T2IF = 0;      // clear interrupt flag
    _T2IE = 1;      // enable TMR2 interrupt
    OC2R = OC2RS = 0;     // initat 50% duty cycle
                            // OC1R also loaded since first time.
    OC2CON = 0x0006;    // OCTSEL = 0 for Timer2, OCM<2:0> =110 for PWM mode
    
}//InitStepper

void _ISR _T4Interrupt(void)
{
    dSec++;
    if(dSec > 9)
    {
        dSec = 0;
        Sec++;
        if(Sec > 59)
        {
            Sec = 0;
            Min++;
            if(Min > 59)
            {
                Min = 0;
                Hr++;
                if(Hr > 23)
                {
                    Hr = 0;
                }
            }
        }
    }
    _T4IF = 0;
}

void StepTrash()
{
    int curSec = Sec;
    MoveStep270();
    while(StepCheckTime(curSec));
    MoveStep90();
}

void StepMetal()
{
    ;
}

void InitTime(void)
{
    _T4IP = 1;
    TMR4 = 0;
    T4CON = 0x8030;
    PR4= 6250 - 1;
    _T4IF = 0;
    _T4IE = 1;

}//InitTime

void StepGlass()
{
    int curSec = Sec;
    MoveStep180();
    while(StepCheckTime(curSec));
    MoveStep180();
}

void I2Cinit(int BRG)
{
    I2C2BRG = BRG;
    while (I2C2STATbits.P); //Check if bus is idle
    I2C2CONbits.A10M = 0; //Select 7-bit address mode
    I2C2CONbits.I2CEN = 1; //I2C Enable
}//I2Cinit

void I2CStart(void)
{
    us_delay(20);
    I2C2CONbits.SEN = 1; //Initiates start condition on SDAx and SCLx pins
    while (I2C2CONbits.SEN); //Await start condition
    us_delay(20);
}//I2CStart

void I2CStop(void)
{
    us_delay(20);
    I2C2CONbits.PEN = 1; //Halts condition enable bit
    while (I2C2CONbits.PEN); //Await stop conditon
    us_delay(20);
}//I2CStop

char I2Cgetbyte(void)
{
    I2C2CONbits.RCEN = 1; //Enable I2C receive mode
    while (!I2C2STATbits.RBF); //Wait for byte to shift into I2C1RCV register
    I2C2CONbits.ACKEN = 1; //Master sends acknowledge
    us_delay(20);
    return (I2C2RCV);
}//I2Cgetbyte

void I2Csendbyte(char data) {
    while (I2C2STATbits.TBF); //wait if buffer is full
    I2C2TRN = data; // pass data to transmission register
    us_delay(10); // delay to be safe
}//I2Csendbyte

void us_delay(int N)
{
    T1CON = 0x8000;
    int u_delay = 16;
    while (TMR1 < u_delay * N)
    {
    }
    TMR1 = 0;
}//us_delay

void InitServo(void) {
    T3CON = 0x8030; // enable TMR3, 1:256, 16 bit Timer, intclock
    PR3 = 1250 - 1;  // 20 kHz for Lab5. see Equ. 14-1 of data sheet, Fcy=16MHz.
    _T3IF = 0;      // clear interrupt flag
    _T3IE = 1;      // enable TMR3 interrupt
    OC1R = OC1RS = 625;     // initat 50% duty cycle
                            // OC1R also loaded since first time.
    OC1CON = 0x000E;    // OCTSEL = 1 for Timer3, OCM<2:0> =110 for PWM mode
} // InitAudio

void _ISRFAST _T3Interrupt(void)
{
    if (SERVO_MOVE)		// Move back
    {
        OC1RS = SERVO_BACK;
    }
    else
    {

        OC1RS = SERVO_FORWARD;
    }
    _T3IF = 0;      // clear interrupt flag and exit
} // T3 Interrupt



void InitADC(void)
{
    AD1CON1 = 0x00E0;   //auto-convert mode
    AD1CON2 = 0;        //using MUXA, AVss and AVdd as Vref
    AD1CON3 = 0x1F01;   //Tad = 2 * Tcy = 125ns
    AD1CSSL = 0;        //no scanning
    AD1CON1bits.ADON = 1; //activate ADC 
}//InitADC

int StepCheckTime(int curSec)
{
    return !(Sec!= curSec&&Sec%STEP_TIME_DELAY - curSec%STEP_TIME_DELAY == 0);
}

void StepPlastic ()
{
    int curSec = Sec;
    MoveStep90();
    while(StepCheckTime(curSec));
    MoveStep270();
}
