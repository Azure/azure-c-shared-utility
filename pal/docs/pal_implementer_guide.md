# Physical Abstraction Layer Implementer's Guide

The Azure IoT C SDK's [Physical Abstraction Layer](https://github.com/Azure/azure-c-shared-utility/tree/pal/pal)  **TODO: link->master** is designed to make writing a new PAL implementation as simple as possible.

### Argument checking

It is not necessary to do validate incoming arguments when implementing PAL functions. Argument checking has already been performed by the framework before calling the PAL functions, and need not be repeated.
