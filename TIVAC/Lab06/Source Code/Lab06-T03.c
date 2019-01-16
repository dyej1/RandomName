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
#define resolution 0.5

int main(void)
{
    volatile uint32_t ui32Load;
    volatile uint32_t ui32PWMClock;
    volatile float fDutyCycleR;
    volatile float fDutyCycleG;
    volatile float fDutyCycleB;

    //  Begin with 90% duty cycle
    fDutyCycleR = 90;
    fDutyCycleG = 90;
    fDutyCycleB = 90;

    // Clock at 40MHz
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);
    // PWM at 625kHz
    ROM_SysCtlPWMClockSet(SYSCTL_PWMDIV_64);

    // Enable PWM and LEDs
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Use PWM0 and set LEDs to PWM pins
    ROM_GPIOPinTypePWM(GPIO_PORTD_BASE, GPIO_PIN_0);
    ROM_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    // Configure red, green, and blue LEDs to PWM pins
    ROM_GPIOPinConfigure(GPIO_PF1_M1PWM5);
    ROM_GPIOPinConfigure(GPIO_PF2_M1PWM6);
    ROM_GPIOPinConfigure(GPIO_PF3_M1PWM7);

    // Set to 55Hz
    ui32PWMClock = SysCtlClockGet() / 64;
    ui32Load = (ui32PWMClock / PWM_FREQUENCY) - 1;

    // Configure PWM as a down-counter. GEN_2 handles red LED, and GEN_3 handles the blue and green
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
    // Load clock value
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, ui32Load);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, ui32Load);

    // Set PWM resolution
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, fDutyCycleR / 100 * ui32Load);
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, fDutyCycleB / 100 * ui32Load);
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, fDutyCycleG / 100 * ui32Load);

    // Set PWM as output and begin cycle
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_3);

    while(1)
    {
        // Cycle through each LED fading
        for (fDutyCycleR = 90; fDutyCycleR > 9; fDutyCycleR-=resolution)
            for (fDutyCycleG = 90; fDutyCycleG > 9; fDutyCycleG-=resolution)
                    for (fDutyCycleB = 90; fDutyCycleB > 9; fDutyCycleB-=resolution)
                    {
                        ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, fDutyCycleR / 100 * ui32Load);
                        ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, fDutyCycleG / 100 * ui32Load);
                        ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, fDutyCycleB / 100 * ui32Load);
                        ROM_SysCtlDelay(100000);
                    }
    }
}
