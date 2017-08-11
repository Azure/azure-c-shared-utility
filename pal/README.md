# Physical Abstraction Layer

The **Physical Abstraction Layer (PAL)** of the 
[Azure IoT C Shared Utility library](https://github.com/Azure/azure-c-shared-utility/tree/master) 
abstracts all of the various hardware platforms and SSL implemnentations that the 
[Azure IoT SDKs](https://github.com/Azure/azure-iot-sdks) can work with.

The Physical Abstraction Layer replaces adapters written using the [older porting guide](https://github.com/Azure/azure-c-shared-utility/blob/master/doc/porting_guide.md), and is designed to

* Simplify the writing of TLS adapters by providing a reusable state machine.
* Simplify re-use of existing code by providing a one-stop [PAL Example Finder]()