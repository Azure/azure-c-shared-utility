# Required functions for porting

This file demonstrates the required OS functions required for porting the Azure IoT SDK to a new device.

```c
// Required Azure IoT headers
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/agenttime.h"

// TODO: Add includes for your device here

// Sleep the current thread for the specified milliseconds
void ThreadAPI_Sleep(unsigned int milliseconds)
{
    // An example from FreeRTOS
    vTaskDelay((milliseconds * configTICK_RATE_HZ) / 1000);
}

// This function must return seconds since Jan 1, 1970. Nearly all
// implementations of the C time function provide this value
// in seconds. However, if your implementation does not,
// you must convert your available time to seconds since Jan 1, 1970
time_t get_time(time_t* p)
{
    return time(p);
}

// The standard C function works fine here if available
double get_difftime(time_t stopTime, time_t startTime)
{
    return difftime(stopTime, startTime);
}

// This function is only used for logging, so for highly
// constrained devices you may choose to omit logging and
// this function.
char* get_ctime(time_t* timeToGet)
{
    return ctime(timeToGet);
}


```