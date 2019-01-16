#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#define TARGET_IS_BLIZZARD_RB1
#include "driverlib/timer.h"
#include "driverlib/rom.h"

void UARTIntHandler(void)
{
    uint32_t ui32Status;
    uint8_t backspace;

    ui32Status = UARTIntStatus(UART0_BASE, true); //get interrupt status
    UARTIntClear(UART0_BASE, ui32Status); //clear the asserted interrupts

    // Save the current UART input
    char inputChar;
    inputChar = UARTCharGetNonBlocking(UART0_BASE);


    // When 'R' is input, turn on red LED
    if (inputChar == 'R')
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 2);
    }

    // When 'r' is input, turn off red LED
    else if (inputChar == 'r')
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, 0);
    }

    // When 'G' is input, turn on green LED
    else if (inputChar == 'G')
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 8);
    }

    // When 'g' is input, turn off green LED
    else if (inputChar == 'g')
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, 0);
    }

    // When 'B' is input, turn on blue LED
    else if (inputChar == 'B')
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 4);
    }

    // When 'b' is input, turn off blue LED
    else if (inputChar == 'b')
    {
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0);
    }

    // When 'T' is input, display temperature in console
    else if (inputChar == 'T')
    {
        // Erase any previous "Fahrenheit: ##"
        for (backspace = 0; backspace < 15; backspace++)
            UARTCharPut(UART0_BASE, '\b');

        UARTCharPut(UART0_BASE, 'F');
        UARTCharPut(UART0_BASE, 'a');
        UARTCharPut(UART0_BASE, 'h');
        UARTCharPut(UART0_BASE, 'r');
        UARTCharPut(UART0_BASE, 'e');
        UARTCharPut(UART0_BASE, 'n');
        UARTCharPut(UART0_BASE, 'h');
        UARTCharPut(UART0_BASE, 'e');
        UARTCharPut(UART0_BASE, 'i');
        UARTCharPut(UART0_BASE, 't');
        UARTCharPut(UART0_BASE, ':');
        UARTCharPut(UART0_BASE, ' ');
        UARTCharPut(UART0_BASE, ' ');
        UARTCharPut(UART0_BASE, ' ');

        // Start timer to begin ADC conversion process
        TimerEnable(TIMER1_BASE, TIMER_A);
        IntEnable(INT_TIMER1A);
    }

    // When 't' is input, stop displaying temperature in console
    else if (inputChar == 't')
    {
        // Stop timer to stop ADC conversion process
        TimerDisable(TIMER1_BASE, TIMER_A);
        IntDisable(INT_TIMER1A);

        // Erase any previous "Fahrenheit: ##"
        for (backspace = 0; backspace < 16; backspace++)
        {
            UARTCharPut(UART0_BASE, ' ');
            UARTCharPut(UART0_BASE, '\b');
            UARTCharPut(UART0_BASE, '\b');
        }
    }
}

    int main(void) {
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
    (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    IntMasterEnable();
    IntEnable(INT_UART0);
    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

    // Enable clock usage for pin F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    // Set the 3 on-board LEDs to outputs
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    // Use to define the period of ADC conversion
    uint32_t ui32Period;

    // Enable ADC Peripheral
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    // Average 64 values before putting in the ADC FIFO
    ROM_ADCHardwareOversampleConfigure(ADC0_BASE, 64);

    // Configure ADC sequencer to use ADC0 and SS1
    ROM_ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);

    // Setup steps for ADC sequencer. The temp value will show the average of 4 temp values
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_TS);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_TS);
    ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);

    // Sample temp again and enable the interrupt flag to say that calculation has been completed
    ROM_ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_TS|ADC_CTL_IE|ADC_CTL_END);

    // Enable ADC sequence
    ROM_ADCSequenceEnable(ADC0_BASE, 1);

    // Enable timer1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);

    // Clock period of 2Hz
    ui32Period = SysCtlClockGet() / 2;
    TimerLoadSet(TIMER1_BASE, TIMER_A, ui32Period -1);

    // Enable timer1 overload as interrupts
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();

    while (1)
    {
    }
}

void Timer1IntHandler(void)
{
    UARTCharPut(UART0_BASE, '\b');
    UARTCharPut(UART0_BASE, '\b');

    int Tens;
    int Ones;

    // Define variable to store data from the ADC FIFO
    uint32_t ui32ADC0Value[4];

    // Define variables for use in later calculations
    volatile uint32_t ui32TempAvg;
    volatile uint32_t ui32TempValueC;
    volatile uint32_t ui32TempValueF;

    // Clear ADC calculation complete interrupt
    ROM_ADCIntClear(ADC0_BASE, 1);

    // Start ADC conversion
    ROM_ADCProcessorTrigger(ADC0_BASE, 1);

    // Wait for conversion to complete
    while(!ROM_ADCIntStatus(ADC0_BASE, 1, false))
    {
    }

    // Copy ADC value
    ROM_ADCSequenceDataGet(ADC0_BASE, 1, ui32ADC0Value);

    // Calculate average temperature reading
    ui32TempAvg = (ui32ADC0Value[0] + ui32ADC0Value[1] + ui32ADC0Value[2] + ui32ADC0Value[3] + 2)/4;

    // Convert temp to Celsius
    ui32TempValueC = (1475 - ((2475 * ui32TempAvg)) / 4096)/10;

    // Convert temp to Fahrenheit
    ui32TempValueF = ((ui32TempValueC * 9) + 160) / 5;

    Tens = ui32TempValueF - ui32TempValueF % 10;
    Ones = ui32TempValueF - Tens;

    UARTCharPut(UART0_BASE, Tens / 10 + '0');
    UARTCharPut(UART0_BASE, Ones + '0');
}
