# pal_device

## Overview

The **pal_device** component is a part of the Azure IoT C SDK's [Physical Abstraction Layer](https://github.com/Azure/azure-c-shared-utility/tree/pal/pal) **TODO: link->master**.

The purpose of pal_device is to perform any hardware-level initialization that may be needed. Windows, Linux, and Mac will not need to perform such initialization, so their implementations of pal_device will be empty. 

## References

[pal_device.h](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/include/pal_device.h) **TODO: link->master** <br/>
[PAL initialization sequence](https://github.com/Azure/azure-c-shared-utility/tree/pal/pal#component-initialization) **TODO: link->master**

## API Calls

### pal_device_init

```
int pal_device_init();
```

This function must set up any device-level configuration that is needed. Examples include pinout and IRQ assignment. The `pal_device_init` call is always the first initialization to be performed.

Returns 0 on success, non-zero otherwise.

### pal_device_deinit

```
void pal_device_deinit();
```

This function performs any final cleanup that should be done before an orderly shutdown. This call is rarely necessary, and will usually be empty.
