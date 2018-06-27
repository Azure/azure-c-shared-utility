# How to Port the Azure IoT C SDK  to Other Platforms

This document shows how to port the C Shared utility library to 
platforms not supported out of the box.
The C shared utility library is used by C SDKs like IoTHub client SDK and EventHub client SDK.  

## References

###### Specifications
- [threadapi and sleep adapter specification](https://github.com/Azure/azure-c-shared-utility/blob/master/devdoc/threadapi_and_sleep_requirements.md)<br/>
- [lock adapter specification](https://github.com/Azure/azure-c-shared-utility/blob/master/devdoc/lock_requirements.md)<br/>



## Overview

The C shared utility library is written in C99 for the purpose of portability to most platforms.

To port the C shared utility library to a new platform, you'll need to
1. Set your project to use include the common porting files and headers
2. Implement the required platform functions
3. If necessary, implement the optional platform threading functions
4. Identify your OS and device architecture
5. Implement the functions necessary for TLS communication

The following sections describe each step in detail.

### Step 1 &ndash; Include common porting files and headers

The common porting source files are found in the `pal` directory. Include these two files:
* `tlsio_adapter_common.c`
* `tlsio_options.c`

The headers for the common porting files are found in the `pal` directory. Add these two
paths to your project header include directories:
* `pal/inc`
* `pal/generic`


### Step 2 &ndash; Implement the required platform functions

The SDK needs a small set of functions implemented to adapt to your device. To implement these
functions, create a file named `azure_iot_port.c` (or whatever you prefer), and add it to
your project. Then copy the code from [this page](porting_example_required.md) into your
`azure_iot_port.c` file and modify the functions as necessary to work with your device.

### threadapi and lock adapters

The **threadapi** and **lock** adapters must exist for the SDK to compile, but their functionality is optional.
Their specification documents (see below) detail what each empty function should do if threading functionality
is not needed.

These components that allow the SDK to 
communicate with an IoT Hub within a dedicated thread. The use of a dedicated thread has some cost 
in memory consumption because of the
need for a separate stack, which may make the dedicated thread difficult to use on devices with little free
memory. 

The upside of the dedicated thread is that all current tlsio adapters may repeatedly
block for some fraction 
of a minute when attempting to connect to their IoT Hub when the network is unavailable, and having a thread
dedicated to the Azure IoT SDK will allow other device functions to remain responsive while the SDK is blocked.

Future versions of the SDK may eliminate this potential blocking behavior, but for now, devices which must
be responsive will need to use a dedicated thread for the SDK, which requires implementing the ThreadApi
and Lock adapters.

Here are the specs for the threadapi and lock adapters:
- [threadapi and sleep adapter specification](https://github.com/Azure/azure-c-shared-utility/blob/master/devdoc/threadapi_and_sleep_requirements.md)<br/>
- [lock adapter specification](https://github.com/Azure/azure-c-shared-utility/blob/master/devdoc/lock_requirements.md)<br/>

These specs explain how to create null _lock_ and _threadapi_ adapters for when threading is not desired.

To get started creating your threadapi and lock adapters, copy these Windows adapter files and modify
them appropriately:
- [threadapi.c](https://github.com/Azure/azure-c-shared-utility/blob/master/adapters/threadapi_win32.c)
- [lock.c](https://github.com/Azure/azure-c-shared-utility/blob/master/adapters/lock_win32.c)

