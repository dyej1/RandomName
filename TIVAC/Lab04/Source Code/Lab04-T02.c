#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "inc/hw_gpio.h"

int main(void)
{
    // Use to define the period of LED blinking
    uint32_t ui32Period;

    // Used to generate waveform graph of LED
    uint32_t BlueLED;

    // Set clock to 40MHz
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    // Enable LED's as outputs
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    // Unlock GPIOCR for register F
    HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    // Free up sw2
    HWREG(GPIO_PORTF_BASE + GPIO_O_CR) = 0x01;

    // Set sw2 to be an input
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);

    // Enable sw2 as an interrupt
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0);
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_INT_PIN_0, GPIO_RISING_EDGE);
    IntEnable(INT_GPIOF);

    // Enable timer0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

    // Clock period of 2Hz with 75% duty cycle
    ui32Period = (SysCtlClockGet() / 2) * 3/4;
    TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);

    // Enable timer0 overload as interrupts
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();

    // Start timer
    TimerEnable(TIMER0_BASE, TIMER_A);

    while(1)
    {
        // Store the value of the blue LED
        BlueLED = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2);
    }
}

// When timer0 overloads, cycle the LED
void Timer0IntHandler(void)
{
    // Redefined for use in interrupt handler
    uint32_t ui32Period;

    // Clear the timer interrupt
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    // Read the current state of the GPIO pin and
    // write back the opposite state
    if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2))
    {
        // Turn off LEDs
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

        // Set timer0 to have 25% duty cycle while LED is off
            ui32Period = (SysCtlClockGet() / 2)* 1/4;
            TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
    }
    else
    {
        // Turn on blue LED
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);

        // Set timer0 to have 75% duty cycle while LED is on
            ui32Period = (SysCtlClockGet() / 2)* 3/4;
            TimerLoadSet(TIMER0_BASE, TIMER_A, ui32Period -1);
    }

}


// When sw2 is pushed, hold for 1.5 sec
void PortFIntHandler(void)
{
    // Disable timer0 interrupt, but keep timer0 running for later use
    TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Clear sw2 interrupt
    GPIOIntClear(GPIO_PORTF_BASE, GPIO_INT_PIN_0);

    // If sw2 has been pressed, turn on the blue LED
    if (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0))
    {   // Turn on blue LED
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2,4);

        // Enable timer1
        SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
        TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

        // Setup timer1 as 2/3Hz (3/2sec)
        uint32_t ui32Period = (SysCtlClockGet()/2*3);
        TimerLoadSet(TIMER1_BASE, TIMER_A, ui32Period -1);

        // Enable overload of timer1 as interrupt
        IntEnable(INT_TIMER1A);
        TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
        IntMasterEnable();

        // Start timer1
        TimerEnable(TIMER1_BASE, TIMER_A);
    }
}

// When timer1 overloads, go back to oscillating with timer0
void Timer1IntHandler(void)
{
    // Clear the timer interrupt
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // Turn off timer1 and disable its interrupt
    TimerDisable(TIMER1_BASE, TIMER_A);
    IntDisable(INT_TIMER1A);

    // Turn off LED to set to beginning of blinking cycle
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);

    // Re-enable timer0 overload interrupt to restart blinking cycle
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}
