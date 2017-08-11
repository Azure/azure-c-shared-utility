# Physical Abstraction Layer

The **Physical Abstraction Layer (PAL)** of the 
[Azure IoT C Shared Utility library](https://github.com/Azure/azure-c-shared-utility/tree/master) 
provides a consistent abstraction for the various hardware platforms and SSL implemnentations that the 
[Azure IoT SDKs](https://github.com/Azure/azure-iot-sdks) can work with.

The Physical Abstraction Layer replaces adapters written using the [older porting guide](https://github.com/Azure/azure-c-shared-utility/blob/master/doc/porting_guide.md), and is designed to

* Simplify TLS implementation by providing a reusable state machine.
* Simplify re-use of existing code by providing a one-stop [PAL Example Finder](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/example_finder.md) to help implementors find existing solutions.
* Aid development and assure quality by providing a [PAL Test Suite](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/tests/README.md) that verifies proper operation of a PAL implementation.
* Move responsibility for new device implementation and support out of the SDK and into the hands of device vendors.
* Provide CMAKE-based tools to help vendors consume the SDK within vendor build systems (such as Arduino) that may have inflexible constraints on file arrangement.
* Add explicit organization of common constrained-device initialization functions such as pin assignment, WiFi setup, and NTP service setup. This will allow the writing of samples for constrained devices which have no device-dependent code, thus eliminating redundant, conflicting, and easily broken initialization code in the SDK samples.


