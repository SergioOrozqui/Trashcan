#include "config.h"



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