#include "config.h"

void I2Cinit(int BRG)
{
    I2C1BRG = BRG;
    while(I2C1STATbits.P); //Check if bus is idle
    I2C1CONbits.A10M = 0; //Select 7-bit address mode
    I2C1CONbits.I2CEN = 1; //I2C Enable
}

void I2CStart(void)
{
    us_delay(20);
    I2C1CONbits.SEN = 1; //Initiates start condition on SDAx and SCLx pins
    while(I2C1CONbits.SEN); //Await start condition
    us_delay(20);
}

void I2CStop(void)
{
    us_delay(20);
    I2C1CONbits.PEN = 1; //Halts condition enable bit
    while(I2C1CONbits.PEN); //Await stop conditon
    us_delay(20);
}

char I2Cgetbyte(void)
{
    I2C1CONbits.RCEN = 1; //Enable I2C receive mode
    while(!I2C1STATbits.RBF); //Wait for byte to shift into I2C1RCV register
    I2C1CONbits.ACKEN = 1; //Master sends acknowledge
    us_delay(20);
    return (I2C1RCV);
}

void InitPWM(void) {
    T3CON = 0x8000; // enable TMR3, 1:1, 16 bit Timer, intclock
    PR3 = 800 - 1;  // 20 kHz for Lab5. see Equ. 14-1 of data sheet, Fcy=16MHz.
    _T3IF = 0;      // clear interrupt flag
    _T3IE = 1;      // enable TMR3 interrupt
    OC1R = OC1RS = 400;     // initat 50% duty cycle
                            // OC1R also loaded since first time.
    OC1CON = 0x000E;    // OCTSEL = 1 for Timer3, OCM<2:0> =110 for PWM mode
} // InitAudio

void _ISRFAST _T3Interrupt( void)
{
   OC1RS = 400;		// Count is set in main()                // increment k each Timer interrupt
   _T3IF = 0;      // clear interrupt flag and exit
} // T3 Interrupt

int main (void)
{
    InitPWM();
    while(1);
    return 0;
}
