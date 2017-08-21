# pal_os

## Overview

The **pal_os** component is a part of the Azure IoT C SDK's [Physical Abstraction Layer](https://github.com/Azure/azure-c-shared-utility/tree/pal/pal) **TODO: link->master**.

The pal_os component exposes device-independent time, sleep, and tickcounter functions to the Azure IoT C SDK. All of the  functions in pal_os are mandatory. 

The functions in pal_os may require some device setup (such as starting an NTP service and waiting for the correct time to be set). If such initialization is necessary it must be performed by the application before connecting to an Azure IoT Hub. For sample programs, this initialization for each device should go into the [pal_sample_runner](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/docs/pal_sample_runner.md) **TODO: link->master** component.

## References

[pal_os.h](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/include/pal_os.h) **TODO: link->master** <br/>

## API

### tickcounter types

```
// Tickcounter vlues are typedef'd as uint32_t instead of uint_fast32_t
// so that native 32 bit tickcounters can always be used directly, even
// if the app is compiled for 64 bit integers.
typedef uint32_t tickcounter_count_ms_t;
typedef uint32_t tickcounter_interval_ms_t;
```
The tickcounter value type and the tickcounter interval type are represented separately to emphasize that their semantics are different despite their identical underlying type.

### pal_tickcounter_get_count_ms
```
// Return the current value of the free-running counter.
tickcounter_count_ms_t pal_tickcounter_get_count_ms();
```

This function is required for the Azure IoT C SDK to handle timeouts. The PAL implementer must guarantee that this function is available and that its timer is running before asking the Azure IoT C SDK to connect to an Azure IoT Hub. In the vast majority of cases this will simply mean returning a properly scaled version one of the device's native tick counters. 

Returns the value of a free-running timer scaled to milliseconds. The tick quantum may be much larger than one millisecond; a one-second quantum is acceptable, which means that the C `time()` function or a similar RTC value may be used if the device's high-resolution timers are unavailable.

### pal_tickcounter_get_interval_ms

```
tickcounter_interval_ms_t pal_tickcounter_get_interval_ms(tickcounter_count_ms_t previous_count);
```
This function is implemented in the SDK, and PAL implementers do not need to implement it.

It returns the time interval between 'now' and the supplied previous tick count. This function is not strictly necessary, but it is cheap insurance against nearly impossible-to-catch underflow errors that might be caused by unnoticed integer promotion.

### pal_get_time
```
time_t pal_get_time(time_t* p);
```
This is a device-independent version of the C `time()` function. The Azure IoT C SDK uses it for cert validity checking and for reporting date-time values. The PAL is agnostic about whether this is local time or UTC.

### pal_thread_sleep
```
void pal_thread_sleep(unsigned int milliseconds);
```
Causes the current thread to sleep for at least the supplied number of milliseconds. Exact behavior is platform-dependent.

