# How to Port the Azure IoT C SDK  to Other Platforms

This document shows how to port the C Shared utility library to 
platforms not supported out of the box.
The C shared utility library is used by C SDKs like IoTHub client SDK and EventHub client SDK.  

## References

###### Specifications
- [threadapi and sleep adapter specification](threadapi_and_sleep_requirements.md)<br/>
- [lock adapter specification](lock_requirements.md)<br/>


## Overview

The C shared utility library is written in C99 for the purpose of portability to most platforms.

To port the C shared utility library to a new platform, you'll need to
1. Set your project to use include the common porting files and headers
2. Implement the required platform functions
3. If necessary, implement the optional platform threading functions
4. Identify your OS and device architecture
5. If desired, implement custom Azure IoT logging for your device
6. Implement the functions necessary for TLS communication

The following sections describe each step in detail.

### Step 1 &ndash; Include common porting files and headers

The common porting source files are found in the C shared utility library's 
`pal` directory. Include these three files:
* `tlsio_adapter_common.c`
* `tlsio_options.c`
* `tickcounter.c`

The headers for the common porting files are found in the `pal` directory. Add these two
paths to your project header include directories:
* `pal/inc`
* `pal/generic`


### Step 2 &ndash; Implement the required platform functions

The SDK needs a small set of functions implemented to adapt to your device. To implement these
functions, create a file named `azure_iot_port.c` (or whatever you prefer), and add it to
your project. Then copy the code from [this template](porting_example_required.md) into your
`azure_iot_port.c` file and modify the functions as necessary to work with your device.

### Step 3 &ndash; Implement optional platform threading functions

It is rarely necessary to implement the platform threading functions. 
The platform threading functions are designed to be used in
larger systems like Windows, Linux, and Apple OSX/iOS, and do not lend themselves to 
constrained devices. The SDK already contains platform threading implementations for the 
larger systems.

If you do need to implement the platform threading functions, copy and modify the code from 
[this template](porting_example_threading.md) into an `azure_iot_port_threading.c` file 
and include it in your project.

More info about implementing the platform threading functions can be found in in:
- [threadapi and sleep adapter specification](threadapi_and_sleep_requirements.md)<br/>
- [lock adapter specification](lock_requirements.md)<br/>

**Important note for constrained devices:** The Azure IoT C SDK contains several files 
which occur in pairs, where there is
a `filename.c` and a `filename_ll.c` The `_ll` means "low-level", which means that
the non-`_ll.c` file requires the platform threading functions, while the `_ll.c` 
version does not require the platform threading functions. So if your linker complains about 
missing any of the platform threading functions, make sure you are not including any of the non-`_ll.c`
files in your project, and make all of your SDK calls to the `_ll` function variants instead.

### Step 4 &ndash; Identify your OS and device architecture

The Azure IoT SDK needs to identify both your OS and your device architecture to 
the Azure IoT Hub. This is done with the preprocessor defines `AZURE_IOT_OS_USER_AGENT` 
and `AZURE_IOT_ARCH_USER_AGENT`.

To identify your OS, create an `azure_iot_os_user_agent.h` file and include it in your
project. Here is an example from FreeRTOS that pulls in the OS version number:
```c
#ifndef AZURE_IOT_OS_USER_AGENT_H_
#define AZURE_IOT_OS_USER_AGENT_H_

#include "FreeRTOS.h"
#include "task.h"

// A string reporting the OS in the Azure IoT user agent string
// This is used in the platform_get_platform_info() call
#define AZURE_IOT_OS_USER_AGENT "FreeRTOS " tskKERNEL_VERSION_NUMBER

#endif /* AZURE_IOT_OS_USER_AGENT_H_ */
```

To identify your device architecture, create an `azure_iot_arch_user_agent.h` file
and include it in your project. Here is an example for the LPC43xx chip family:
```c
#ifndef INC_AZURE_IOT_ARCH_USER_AGENT_H_
#define INC_AZURE_IOT_ARCH_USER_AGENT_H_

// A string reporting the architecture in the Azure IoT user agent string
// This is used in the platform_get_platform_info() call
#define AZURE_IOT_ARCH_USER_AGENT "LPC43xx"

#endif /* INC_AZURE_IOT_ARCH_USER_AGENT_H_ */
```

### Step 5 &ndash; If desired, implement custom Azure IoT SDK logging

Constrained devices often have special requirements for logging. If this is the
case, you can omit the Azure IoT SDK's `xlogging.c` file from your project
and replace it with a custom logger. 
[Here is a custom logger template](porting_example_logging.md) 
that you can modify for this purpose.

### Step 6 &ndash; Implement TLS communication

The Azure IoT Hub requires secure communication over TLS. The Azure IoT SDK uses
a layered architecture that makes it simple to implement TLS for new devices.

If you're curious about the layered architecture, it is discussed 
[in this overview](tlsio_adapter_overview.md).

Details of how to implement TLS communication are in 
[this implementers guide](tls_adapter_implementers_guide.md).

