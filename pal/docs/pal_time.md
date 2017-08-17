# pal_time

## Overview

The **pal_time** component is a part of the Azure IoT C SDK's [Physical Abstraction Layer](https://github.com/Azure/azure-c-shared-utility/tree/pal/pal) **TODO: link->master**.

The purpose of pal_time is to perform NTP service initialization that may be necessary and to expose a device-independent version of the C `time()` function to the Azure IoT SDK. 

The Azure IoT C SDK uses this component to check certificate expiration times and to report the time of certain events.

## References

[pal_time.h](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/include/pal_time.h) **TODO: link->master** <br/>
[PAL initialization sequence](https://github.com/Azure/azure-c-shared-utility/tree/pal/pal#component-initialization) **TODO: link->master**

## API Calls

### pal_time_init

```
int pal_time_init(bool blocking);
```

This function must initialize the time-setting service. If the `blocking` parameter is `true`, then pal_time_init shall block until the system time has been set to a reasonable value, where "reasonable" is defined by the PAL implementer. "Reasonable" does not have to mean that the NTP server has been contacted; if the device has an appently good time in its RTC left over from a previous boot cycle, the PAL implementation is free to declare that good enough. As far as the Azure IoT SDK is concerned, "reasonable" means that the time is good enough to check TLS certificate expiration, and that's a pretty low bar.

The common usage of this call will be with `blocking` true. The non-blocking version will only be needed if there is other prep work the device could be doing while waiting for the NTP server to answer the phone.

Returns 0 on success, non-zero otherwise.

### pal_time_deinit

```
void pal_device_deinit();
```

This function performs any final cleanup that should be done before an orderly shutdown. This call is rarely necessary, and will usually be empty.

### pal_time_is_reliable

```
bool pal_time_is_reliable();
```
This function returns `true` if the system time is "reasonable" by the criteria described in [pal_time_init](#pal_time_init). Connection to Azure IoT hubs may fail due to certificate expiration issues if this function has not yet returned `true`, but the SDK does not check this. It is the responsibility of the app designer to ensure that the system time is reliable before attempting to connect to an Azure IoT Hub.

If `pal_time_init` is called with `blocking` true, then this function will not be needed.

### pal_get_time

```
time_t pal_get_time(time_t* p);
```

A device-independent version of the C time() function.


