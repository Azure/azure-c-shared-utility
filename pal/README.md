# Physical Abstraction Layer

The **Physical Abstraction Layer (PAL)** of the 
[Azure IoT C Shared Utility library](https://github.com/Azure/azure-c-shared-utility/tree/master) 
provides a consistent abstraction for the various hardware platforms and SSL implemnentations that the 
[Azure IoT SDKs](https://github.com/Azure/azure-iot-sdks) can work with.

### Supported Devices

PAL implementations will be provided within the SDK itself for Windows, Linux, and devices sold by Microsoft as part of a kit. Other PAL implementations will be maintained external to the Azure IoT SDK by third-party implementers. (**Is this correct?**)

### Goals
The Physical Abstraction Layer replaces adapters written using the [older porting guide](https://github.com/Azure/azure-c-shared-utility/blob/master/doc/porting_guide.md), and is designed to:

* Simplify TLS implementation by providing a reusable state machine and simplified API.
* Aid development and assure quality by providing a [PAL Test Suite](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/tests/README.md)**TODO: link->master** that verifies proper operation of a PAL implementation.
* Move responsibility for new device implementation and support out of the SDK and into the hands of device vendors.
* Provide CMAKE-based tools to help vendors consume the SDK within vendor build systems (such as Arduino) that may have inflexible constraints on file arrangement.
* Add a system of reusable of constrained-device initialization modules to handle pin assignment, WiFi setup, and NTP service setup for SDK sample. This is needed to eliminate multiple copies of fragile initialization code in the SDK samples.
* Simplify re-use of existing code by providing a one-stop [PAL Example Finder](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/docs/example_finder.md) **TODO: link->master** to help implementors find existing solutions.
* Encourage memory conservation for cost-sensitive devices.
* Provide a reusable device initialization strategy for samples. 

### Non-goals
The purpose of the Physical Abstraction Layer is to enable the Azure IoT C SDK to work with a wide range of devices, and it must be as easy and bullet-proof as possible to implement. Accordingly,

* Although some PAL functions such as tickcounter may be of general use, the PAL design will be narrowly focused on supporting the SDK. Functionality not required by the SDK will be not included in the PAL.
* The PAL API does not attempt to support multiple PALs in an application.
* The PAL API does not attempt to support connection to multiple IoT Hubs within a single application. This plus the "only one PAL" rule and the "Encourage memory conservation" design goal means that PAL implementations are encouraged to use file-scope static storage rather than heap storage to avoid heap overhead and allocation error checking code.

## Implementer Components

The PAL implementer is responsible for providing and ensuring correct operation of the PAL implementer components.

#### pal_os

The mandatory [pal_os](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/docs/pal_os.md) **TODO: link->master** component exposes device-independent functions for time, sleep, and tickcounter. 

#### pal_tls

The mandatory [pal_tls](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/docs/pal_tls.md) **TODO: link->master** component provides TLS connection to an Azure IoT Hub.

#### pal_socket_async

The mandatory [pal_socket_async](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/docs/pal_socket_async.md) **TODO: link->master** component provides a device-independent socket abstraction to the PAL framework. Built-in implementations of this component are expected to work for nearly all devices by changing only header include paths.

#### pal_dns

The mandatory [pal_dns](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/docs/pal_dns.md) **TODO: link->master** component performs IP address lookups. Built-in implementations of this component are expected to work for nearly all devices by changing only header include paths. Future versions may implement DNS lookup on top of pal_socket_async, in which case this component will become built-in.

#### pal_threading

The optional [pal_threading](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/docs/pal_threading.md) **TODO: link->master** component allows the SDK to communicate with the IoT Hub on a background thread. 

#### pal_sample_runner

The [pal_sample_runner](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/docs/pal_sample_runner.md) **TODO: link->master** component is not used for production applications, but is needed to support the Azure IoT SDK samples.

## Built-in Components

The built-in components of the PAL are part of the SDK, and are maintained by the Azure IoT SDK team. They can be ignored by PAL implementers.

#### pal_socket_async_io

The [pal_socket_async_io](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/docs/pal_socket_async_io.md) **TODO: link->master** component is an `xio` wrapper around pal_socket_async. Because pal_socket_async hides device-dependent details, a single pal_scoket_async_io is able to work unchanged on all devices. It replaces all of the older-style socketio_xxxx adapters.

#### pal_tls_io

The [pal_tls_io](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/docs/pal_tls_io.md) **TODO: link->master** component combines the pal_tls, pal_socket_async, and pal_dns components into a single 'tlsio' component that the Azure IoT C SDK can use to communicate with an Azure IoT Hub.

#### pal_verification_suite

The [pal_verification_suite](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/docs/pal_verification_suite.md) **TODO: link->master** is a development and QA tool that PAL implementers can use to help implement their PAL and verify its correct operation.
