/*Senior Design Project: Smart Recycling Bin
 *main.c
 *The University of Akron
 *Department of Electrical and Computer Engineering
 *Authors: Brandon Bannavong, Sergio Orozco
 *Design Team: 14
 * 
 *Purpose: This program receives and transmits various signals to 
 *and from the PIC24FJ128GA010 microcontroller (PIM) mounted on the
 *Explorer 16/32 development board. Data collected is used to determine
 *positioning and timing for both the stepper motor and servo. Remaining 
 *bin capacity and item classification are tracked and displayed to the user. 
 *Communication with the NVIDIA Jetson Nano (visual detection system) 
 *is performed using the UART communication protocol.
 */

//Include statements
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "lcd.h"

//Define statements
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

//Global variables
char * beginMessage = {"`x"};
char getU2 = 'x';
volatile char recMess[10];
char* types = { "Plastic","Glass","Metal","Trash" };
volatile int steps =0;
volatile int dSec = 0, Sec = 0, Min = 0, Hr = 0;
volatile bool updateLCD = false;
int STEP_MOVE = 0;
int SERVO_MOVE = 0;


//Function prototypes
int StepCheckTime(int );
int ReadADC(int);
void InitADC(void);
void InitServo(void);
void ServoMove(int);
void InitStepper();
void UpdateLCD();
void SendDetectionMsg();

void InitU2();
char PutU2(char c);
char GetU2();
void PutU2String(char *, int);

void _ISRFAST _T2Interrupt();
void _ISRFAST _T3Interrupt(void);
void _ISR _T4Interrupt(void);

void StepPlastic();
void StepGlass();
void StepTrash();
void StepMetal();
void InitTime();


void MoveStep90();
void MoveStep270();
void MoveStep180();

void ms_delay(int);
void us_delay(int i);

struct Item
{
    int weight;
    char* type;
    float volume;
};


//Begin main routine
int main(void)
{

    int c;
    struct Item Item1;
    
    recMess[0]= 'x';
    recMess[1]= 'x';
    recMess[2]= '\0';
    TRISD = 0xFFFF;
    InitADC();
    InitTime();
    InitServo();
    InitStepper();
    InitU2();
    LCD_Initialize();
    LCD_ClearScreen();
    
    while (1)
    {
        if (PORTDbits.RD6 == 0)
        {           
//            PutU2String(beginMessage,3);
//            recMess[0] = GetU2();
//            recMess[1] = GetU2();
//            recMess[2] = GetU2();
            SendDetectionMsg();
            if (recMess[0] = '`')    
            {
                switch(recMess[1])
                {
                        case 'a':
                            StepMetal();
                            Item1.type = types[2];
                            break;

                        case 'p':
                            StepPlastic();
                            Item1.type = types[0];
                            break;

                        case 'g':
                            StepGlass();
                            Item1.type = types[1];
                            break;

                        case 'u':
                            StepPlastic();
                            Item1.type = types[3];
                            break;

                        case 'n':
                            StepTrash();
                            Item1.type = types[3];
                            break;
                }             
            }
        }
        else if(PORTDbits.RD7 == 0)
        {  
            StepGlass();
            //MoveStepQuart();
            //LCD_ClearScreen 
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
        ms_delay(10);
    }
    return 0;
}//main

void SendDetectionMsg(void)
{
    PutU2String(beginMessage,4);
    recMess[0] = GetU2();
    recMess[1] = GetU2();
    recMess[2] = GetU2();
    
}

//Rotate stepper motor 90 degrees
void MoveStep90(void) 
{
    STEP_MOVE = 1;
    while(steps<STEP_QUART);
    STEP_MOVE = 0;
    steps = 0;
}//MoveStepQuart

//Rotate stepper motor 270 degrees
void MoveStep270(void)
{
    MoveStep90();
    MoveStep90();
    MoveStep90();
}//MoveStep270

//Rotate stepper motor 180 degrees
void MoveStep180(void)
{
    MoveStep90();
    MoveStep90();
}//MoveStep180

//Initialize UART communication protocol
void InitU2(void) {
    U2BRG = 34;             // PIC24FJ128GA010 data sheet, 17.1 for calculation, Fcy = 16MHz.
    U2MODE = 0x8008;        // See data sheet, pg148. Enable UART2, BRGH = 1,
                            // Idle state = 1, 8 data, No parity, 1 Stop bit
    U2STA = 0x0400;         // See data sheet, pg. 150, Transmit Enable
                            // Following lines pertain Hardware handshaking
    TRISFbits.TRISF13 = 1;  // enable RTS , output
    RTS = 1;                // default status , not ready to send
}//InitU2

/* Transmit a string using UART
 * Parameters: Character string, integer length of string
 * Returns: Nothing
 */
void PutU2String(char* str, int len)
{
    int c;
    for(c = 0; c<len;c++)
    {
        PutU2(str[c]);
    }
}//PutU2String

/*Transmit a character using UART
 *Parameters: Input character
 *Returns: Input character
 */
char PutU2(char c) {
    while (CTS); //wait for !CTS (active low)
    while (U2STAbits.UTXBF); // Wait if transmit buffer full.
    U2TXREG = c; // Write value to transmit FIFO
    return c;
}//PutU2

/*Receive a character using UART
 *Parameters: None
 *Returns: UART receive buffer
 */
char GetU2(void) {
    RTS = 0; // telling the other side !RTS
    while (!U2STAbits.URXDA); // wait
    RTS = 1; // telling the other side RTS
    return U2RXREG; // from receiving buffer
} //GetU2

//Stepper motor interrupt
void _ISRFAST _T2Interrupt(void)
{   
    if(STEP_MOVE)
    {
        steps++;
        OC2RS = STEP_DUTY; //Set value of output compare register 2
    }
    else 
    {
        OC2RS = 0;
    }
    _T2IF = 0;// clear interrupt flag and exit
} // T2 Interrupt

//Servo interrupt
void _ISRFAST _T3Interrupt(void)
{
    if (SERVO_MOVE)	
    {
        OC1RS = SERVO_BACK;//Set value of output compare register 1
    }
    else
    {

        OC1RS = SERVO_FORWARD;
    }
    updateLCD = true;
    
    _T3IF = 0;//Clear interrupt flag and exit
} // T3 Interrupt

//Timer interrupt 
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
    _T4IF = 0;//Clear interrupt flag
} //T4 Interrupt

//Generate PWM signal for stepper motor
void InitStepper()
{
    T2CON = 0x8030;     //Enable TMR3, 1:256, 16 bit Timer, intclock
    PR2 = 625 - 1;      //100 Hz 
    _T2IF = 0;          //Clear interrupt flag
    _T2IE = 1;          //Enable TMR2 interrupt
    OC2R = OC2RS = 0;   //Initialize at 0% duty cycle
                        //OC2R also loaded since first time.
    OC2CON = 0x0006;    //OCTSEL = 0 for Timer2, OCM<2:0> =110 for PWM mode
    
}//InitStepper

//Direct stepper motor towards trash bin, open servo trapdoor, return both to original position
void StepTrash()
{
    int curSec = Sec;
    MoveStep270();
    ServoMove(1);
    while(StepCheckTime(curSec));
    MoveStep90();
    ServoMove(0);
}//StepTrash

//Direct stepper motor towards glass bin, open servo trapdoor, return both to original position
void StepGlass()
{
    int curSec = Sec;
    MoveStep180();
    ServoMove(1);
    while(StepCheckTime(curSec));
    MoveStep180();
    ServoMove(0);
}//StepGlass

//Direct stepper motor towards plastic bin, open servo trapdoor, return both to original position
void StepPlastic()
{
    int curSec = Sec;
    MoveStep90();
    ServoMove(1);
    while(StepCheckTime(curSec));
    MoveStep270();
    ServoMove(0); 
}//StepPlastic

//Direct stepper motor towards metal bin, open servo trapdoor, return both to original position
void StepMetal()
{
    int curSec = Sec;
    ServoMove(1);
    while(StepCheckTime(curSec));
    ServoMove(0);
    ;
}//StepMetal

//Delay in millisecond increments
void ms_delay(int N)
{
    us_delay(1000 * N);
}//ms_delay

//Delay in microsecond increments
void us_delay(int N)
{
    T1CON = 0x8000; //Enable timer 1
    int u_delay = 16;
    while (TMR1 < u_delay * N)
    {
    }
    TMR1 = 0;       //Reset timer 1
}//us_delay

//Generate PWM for servo
void InitServo(void) {
    T3CON = 0x8030;     //Enable TMR3, 1:256, 16 bit Timer, Initialize clock
    PR3 = 1250 - 1;     //50 Hz 
    _T3IF = 0;          //Clear interrupt flag
    _T3IE = 1;          //Enable TMR3 interrupt
    OC1R = OC1RS = SERVO_BACK; //Initialize at 50% duty cycle
                        //OC1R also loaded since first time.
    OC1CON = 0x000E;    //OCTSEL = 1 for Timer3, OCM<2:0> =110 for PWM mode
} // InitServo

void ServoMove(int i)
{
    SERVO_MOVE = i;
}//ServoMove

//Initialize analog to digital converter
void InitADC(void)
{
    AD1CON1 = 0x00E0;     //Select auto-convert mode
    AD1CON2 = 0;          //Using MUXA, AVss and AVdd as Vref
    AD1CON3 = 0x1F01;     //Tad = 2 * Tcy = 125ns
    AD1CSSL = 0;          //No scanning
    AD1CON1bits.ADON = 1; //Activate ADC 
}//InitADC

/*Read ADC value
 *Parameters: Channel select (integer)
 *Returns: ADC buffer (integer)
 */
int ReadADC(int ch)
{
    AD1CHS = ch;               //Select analog input channel
    AD1CON1bits.SAMP = 1;      //Start sampling.
    while( !AD1CON1bits.DONE); //Wait for conversion to complete
    AD1CON1bits.DONE = 0;      //Clear flag
    return ADC1BUF0;           //Return the ADC buffer
} // ReadADC

//Initialize global timer
void InitTime(void)
{
    _T4IP = 1;      //Set interrupt priority
    T4CON = 0x8030; //Initialize timer 4
    TMR4 = 0;       //Reset timer 4 value
    PR4= 6250 - 1;  //100 Hz
    _T4IF = 0;      //Clear interrupt flag
    _T4IE = 1;      //Enable timer 4
}//InitTime

/*Check current second of global timer (used for delays)
 *Parameters: Integer Sec
 *Returns: Integer when curSec = STEP_TIME_DELAY 
 */
int StepCheckTime(int curSec)
{
    return !(Sec!= curSec&&Sec%STEP_TIME_DELAY - curSec%STEP_TIME_DELAY == 0);
}//StepCheckTime

void UpdateLCD()
{
    int deca = Sec;
    char dopa[5];
    int result;
    
    result = ReadADC(3);
    sprintf(dopa,"%4d",result);
    
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
    LCD_PutString ( dopa,4) ;

    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 'M' ) ;
    LCD_PutChar ( 'e' ) ;
    LCD_PutChar ( 's' ) ;
    LCD_PutChar ( 's' ) ;
    LCD_PutChar ( ':' ) ;
    LCD_PutString ( recMess,2 ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 'y' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( ' ' ) ;
    LCD_PutChar ( 'd' ) ;
    LCD_PutChar ( 'o' ) ;
    LCD_PutChar ( 'm' ) ;
    LCD_PutChar ( 'e' ) ;
    LCD_PutChar ( '!' ) ;
}//UpdateLCD
