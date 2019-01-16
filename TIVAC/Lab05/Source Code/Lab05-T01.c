#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/rom.h"
#include "driverlib/gpio.h"

#ifdef DEBUG
void__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

int main(void)
{
    // Define variable to store data from the ADC FIFO. SS3 has a FIFO depth of 1
    uint32_t ui32ADC0Value[1];

    // Define variables for use in later calculations
    volatile uint32_t ui32TempValueC;
    volatile uint32_t ui32TempValueF;

    // Clock run at 40MHz
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);

    // Enable PortF
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    // Set blue LED to output
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);

    // Enable ADC Peripheral
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    // Average 64 values before putting in the ADC FIFO
    ROM_ADCHardwareOversampleConfigure(ADC0_BASE, 64);

    // Configure ADC sequencer to use ADC0 and SS3
    ROM_ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    // Sample temp and enable the interrupt flag to say that calculation has been completed
    ROM_ADCSequenceStepConfigure(ADC0_BASE,3,0,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);

    // Enable ADC sequence
    ROM_ADCSequenceEnable(ADC0_BASE, 3);

    while(1)
    {
        // Clear ADC calculation complete interrupt
        ROM_ADCIntClear(ADC0_BASE, 3);

        // Start ADC conversion
        ROM_ADCProcessorTrigger(ADC0_BASE, 3);

        // Wait for conversion to complete
        while(!ROM_ADCIntStatus(ADC0_BASE, 3, false))
        {
        }

        // Copy ADC value
        ROM_ADCSequenceDataGet(ADC0_BASE, 3, ui32ADC0Value);

        // Convert temp to Celsius
        ui32TempValueC = (1475 - ((2475 * ui32ADC0Value[0])) / 4096)/10;

        // Convert temp to Fahrenheit
        ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5 + 5;

        // If temp is above 72, turn on LED
        if(ui32TempValueF > 72)
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2,4);
        // If temp is not above 72, turn LED off
        else
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2,0);
    }
}
