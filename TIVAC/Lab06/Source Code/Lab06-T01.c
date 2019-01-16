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

// Used to define the delay between pulse changes
#define delay 150000

int main(void)
{
    volatile uint32_t ui32Load;
    volatile uint32_t ui32PWMClock;
    volatile uint8_t ui8Adjust;
    // Start with center position for 1.5mS pulse
    ui8Adjust = 83;

    // Clock at 40MHz
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    // PWM at 625kHz
    ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

    // Enable PWM
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    // Use PWM0
    ROM_GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
    // Use Pin1 as PWM output
    ROM_GPIOPinConfigure(GPIO_PD0_M1PWM0);

    // Set to 55Hz
    ui32PWMClock = SysCtlClockGet() / 64;
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;
    // Configure PWM as a down-counter
    PWMGenConfigure(PWM1_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
    // Load clock value
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_0, ui32Load);

    // Set PWM resolution
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
    // Set PWM as output
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_0_BIT, true);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_0);

    // Simple delay between calibrating the motor and running the loop
    ROM_SysCtlDelay(100*delay);

    while(1)
    {
        // Count down the pulse width to 800uS
        while(ui8Adjust>43)
        {
            ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
            ROM_SysCtlDelay(delay);
            ui8Adjust--;
        }

        // 44 is the min, so once the value updates to 43, go back to 44
        if(ui8Adjust == 43)
            ui8Adjust++;

        ROM_SysCtlDelay(20*delay);

        // Count up the pulse width to 2200uS
        while(ui8Adjust<122)
        {
            ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_0, ui8Adjust * ui32Load / 1000);
            ROM_SysCtlDelay(delay);
            ui8Adjust++;
        }

        // 121 is the max, so once the loop has been run and the value reaches 122, go back to 121
        if(ui8Adjust == 122)
            ui8Adjust--;

        ROM_SysCtlDelay(20*delay);
    }
}
