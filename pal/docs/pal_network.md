# pal_network

## Overview

The **pal_network** component is a part of the Azure IoT C SDK's [Physical Abstraction Layer](https://github.com/Azure/azure-c-shared-utility/tree/pal/pal) **TODO: link->master**.

The purpose of pal_network is to set up a network connection for the device and optionally connect to a WiFi access point. Windows, Linux, and Mac will not need to perform such initialization, so their implementations of pal_network will be empty. 

## References

[pal_network.h](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/include/pal_network.h) **TODO: link->master** <br/>
[PAL initialization sequence](https://github.com/Azure/azure-c-shared-utility/tree/pal/pal#component-initialization) **TODO: link->master**

## API Calls

### pal_network_init

```
int pal_network_init(const char* ap_ssid, const char* ap_password);
```

This function must perform any initialization that is needed to connect to the internet. If `ap_ssid` is non-NULL then `pal_network_init` must attempt to connect to the specified  WiFi access point.

If `ap_ssid` is non-NULL then `ap_password` is also guaranteed to be non-NULL, although it may be of zero length.

Returns 0 if initialization is successful and connection to the access point (if specified) is established, non-zero otherwise.



### pal_network_deinit

```
void pal_network_deinit();
```

This function performs any cleanup that should be done before an orderly shutdown. This call is rarely necessary, and will usually be empty.
