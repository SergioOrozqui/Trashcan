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

struct item
{
    float weight;
    char* type;
    float volume;

};


#define SERVO_NEUTRAL 94
#define SERVO_FORWARD 38
#define SERVO_BACK 150

#define RTS _RF13 // Output, For potential hardware handshaking.
#define CTS _RF12 // Input, For potential hardware handshaking.

int SERVO_MOVE = 0;
char* types = { "Plastic","Glass","Metal","Trash" };

void InitADC(int input);
void I2Cinit(int BRG);
void I2CStart(void);
void I2CStop(void);
char I2Cgetbyte(void);
void I2Csendbyte(char data);
void us_delay(int i);
void InitPWM(void);
void _ISRFAST _T3Interrupt(void);
void ADCStart();
void ms_delay(int N)
{
    us_delay(1000 * N);
}
void InitU2(void) {
    U2BRG = 415; // PIC24FJ128GA010 data sheet, 17.1 for calculation, Fcy= 16MHz.
    U2MODE = 0x8008; // See data sheet, pg148. Enable UART2, BRGH = 1,
    // Idle state = 1, 8 data, No parity, 1 Stop bit
    U2STA = 0x0400; // See data sheet, pg. 150, Transmit Enable
    // Following lines pertain Hardware handshaking
    TRISFbits.TRISF13 = 1; // enable RTS , output
    RTS = 1; // default status , not ready to send
}
char putU2(char c) {
    while (CTS); //wait for !CTS (active low)
    while (U2STAbits.UTXBF); // Wait if transmit buffer full.
    U2TXREG = c; // Write value to transmit FIFO
    return c;
}

char getU2(void) {
    RTS = 0; // telling the other side !RTS
    while (!U2STAbits.URXDA); // wait
    RTS = 1; // telling the other side RTS
    return U2RXREG; // from receiving buffer
} //getU2

int main(void)
{
    TRISD = 0xFFFF;
    InitPWM();
    InitU2();

    //    I2Cinit(0x9D); //enable I2C
    //    I2CStart(); // initiate start condition
    //    I2Csendbyte(0x64); //begin communication with slave
    //    I2CStop(); // halt condition enable bit

    while (1)
    {
        if (PORTDbits.RD6 == 0)
        {
            SERVO_MOVE = 1;
        }
        else
        {
            SERVO_MOVE = 0;
        }
        putU2('a');
        ms_delay(2);

    }
    return 0;
}



void InitADC(int input)
{
    AD1CHS = input;    //select analog pins
    AD1CON1 = 0x00E0;   //auto-convert mode
    AD1CON2 = 0;        //using MUXA, AVss and AVdd as Vref
    AD1CON3 = 0x1F01;   //Tad = 2 * Tcy = 125ns
    AD1CSSL = 0;        //no scanning
    AD1CON1bits.ADON = 1; //activate ADC 
}

void I2Cinit(int BRG)
{
    I2C2BRG = BRG;
    while (I2C2STATbits.P); //Check if bus is idle
    I2C2CONbits.A10M = 0; //Select 7-bit address mode
    I2C2CONbits.I2CEN = 1; //I2C Enable
}

void I2CStart(void)
{
    us_delay(20);
    I2C2CONbits.SEN = 1; //Initiates start condition on SDAx and SCLx pins
    while (I2C2CONbits.SEN); //Await start condition
    us_delay(20);
}

void I2CStop(void)
{
    us_delay(20);
    I2C2CONbits.PEN = 1; //Halts condition enable bit
    while (I2C2CONbits.PEN); //Await stop conditon
    us_delay(20);
}

char I2Cgetbyte(void)
{
    I2C2CONbits.RCEN = 1; //Enable I2C receive mode
    while (!I2C2STATbits.RBF); //Wait for byte to shift into I2C1RCV register
    I2C2CONbits.ACKEN = 1; //Master sends acknowledge
    us_delay(20);
    return (I2C2RCV);
}

void I2Csendbyte(char data) {
    while (I2C2STATbits.TBF); //wait if buffer is full
    I2C2TRN = data; // pass data to transmission register
    us_delay(10); // delay to be safe
}
void us_delay(int N)
{
    T1CON = 0x8000;
    int u_delay = 16;
    while (TMR1 < u_delay * N)
    {
    }
    TMR1 = 0;
}

void InitPWM(void) {
    T3CON = 0x8030; // enable TMR3, 1:256, 16 bit Timer, intclock
    PR3 = 1250 - 1;  // 20 kHz for Lab5. see Equ. 14-1 of data sheet, Fcy=16MHz.
    _T3IF = 0;      // clear interrupt flag
    _T3IE = 1;      // enable TMR3 interrupt
    OC1R = OC1RS = SERVO_FORWARD;     // initat 50% duty cycle
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
void ADCStart()
{
    AD1CON1bits.SAMP = 1;
    while (!AD1CON1bits.DONE);
    AD1CON1bits.DONE = 0;

}

