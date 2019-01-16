#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/debug.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"

// Using a 55Hz control signal
#define PWM_FREQUENCY 55

// Resolution of duty cycle, defining how long the full LED cycle takes
#define resolution 0.0003

int main(void)
{
    volatile uint32_t ui32Load;
    volatile uint32_t ui32PWMClock;
    volatile float fDutyCycle;
    //  Begin with 10% duty cycle
    fDutyCycle = 10;

    // Clock at 40MHz
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    // PWM at 625kHz
    ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

    // Enable PWM and LEDs
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Use PWM0 and set red LED to output
    ROM_GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
    ROM_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);
    // Use red LED as PWM output
    ROM_GPIOPinConfigure(GPIO_PF1_M1PWM5);

    // Set to 55Hz
    ui32PWMClock = SysCtlClockGet() / 64;
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
    // Configure PWM as a down-counter
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
    // Load clock value
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);

    // Set PWM resolution
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, fDutyCycle / 100 * ui32Load);
    // Set PWM as output
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_2);

    while(1)
    {
        // Reset LED to 10% duty cycle
        fDutyCycle = 10;

        // Count up to 90% duty cycle
        while(fDutyCycle<90+resolution)
        {
            ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, fDutyCycle * ui32Load / 100);

            fDutyCycle = fDutyCycle + resolution;
        }
    }
}
