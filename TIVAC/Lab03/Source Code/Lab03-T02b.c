#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

uint8_t ui8PinData=2;

int main(void)
{
    // Set the clock to 400MHz/2/7 = 28.5MHz
    SysCtlClockSet(SYSCTL_SYSDIV_7|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    // Enable clock usage for pin F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    // Set the 3 on-board LEDs to outputs
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

    while(1)
    {
        // Turn specific LEDs on
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8PinData);
        SysCtlDelay(4000000);
        // Turn all LEDs off
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
        SysCtlDelay(4000000);
        // Update the LED value
        if(ui8PinData==2) {ui8PinData=8;} else {
                    if(ui8PinData==8) {ui8PinData=4;} else {
                        if(ui8PinData==4) {ui8PinData=10;} else {
                            if(ui8PinData==10) {ui8PinData=6;} else {
                                if(ui8PinData==6) {ui8PinData=12;} else {
                                    if(ui8PinData==12) {ui8PinData=14;} else {ui8PinData=2;}}}}}}
    }
}
