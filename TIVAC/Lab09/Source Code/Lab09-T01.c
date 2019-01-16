#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"

// Used to define pi for later use in calculations
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Number of data points used to create the sine wave
#define SERIES_LENGTH 100

// Create a data point array
float gSeriesData[SERIES_LENGTH];

// Counter variable used to count the number of calculated data points
int32_t i32DataCount = 0;

int main(void)
{
    // Used to define the distance between data points in the sine wave
    float fRadians;

    // Enable floating point unit for later use
    ROM_FPULazyStackingEnable();
    ROM_FPUEnable();

    // 50MHz system clock
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // Define the distance in between data points
    fRadians = ((2 * M_PI) / SERIES_LENGTH);

    // Calculate and store all of the desired data point values
    while(i32DataCount < SERIES_LENGTH)
    {
        // Calculated the value for each point on the sine wave
        gSeriesData[i32DataCount] = sinf(fRadians * i32DataCount);
        // Move on to the next data point
        i32DataCount++;
    }

    // Loop indefinitely once the calculation is completed
    while(1)
    {
    }
}
