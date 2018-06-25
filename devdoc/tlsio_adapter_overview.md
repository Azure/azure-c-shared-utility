# tlsio_adapter overview

`tlsio_adapter` is layered set of software components that greatly simplifies the task of adapting new
hardware to work with the Azure IoT C SDK.

The [Azure IoT Porting Guide](porting_guide.md) shows several components called "adapters" which must 
be present to connect 
an unsupported hardware device to the Azure IoT SDK, and all of these are simple to write with the exception 
of the "tlsio" [(specification)](tlsio_requirements.md), which is complex to implement, difficult to verify, and can take 
many weeks of work by even very experienced developers.

The use of use of this new `tlsio_adapter` component supercedes the older tlsio adapter design 
[(specification)](tlsio_requirements.md) 
and speeds the process of developing a tlsio adapter by at least an order of magnitude
via reusable components. 

## References
[xio_requirements](xio_requirements.md)</br>
[xio_adapter_requirements](xio_adapter_requirements.md)</br>
[xio_state_requirements](xio_state_requirements.md)</br>

[xio.h](/inc/azure_c_shared_utility/xio.h)</br>
[xio_adapter.h](/inc/azure_c_shared_utility/xio_adapter.h)</br>
[xio_state.h](/inc/azure_c_shared_utility/xio_state.h)</br>


## The three layers

The `tlsio_adapter` design splits what the porting guide calls 
the "tlsio" into three layers. The first two layers are 
reusable code that is part of the SDK. Only the final layer (at most) needs to be implemented
for new devices. 
* **xio_state** The `xio_state` top layer encapsulates the state handling, error handling,
message queuing, and all but one of the callbacks that are required for an `xio` component. This 
layer is a general solution for `xio` component code reuse; it is not specialized for 
`tlsio` components. The `xio_state` layer is implemented entirely in a single 
file: `xio_state.c`.
*  **tlsio_adapter** The `tlsio_adapter` middle layer encapsulates the remaining 
`xio` callback, error handling,
option handling, platform adapter functions, read buffering, and the validation of 
`xio` creation parameters. The `tlsio_adapter` layer in combination with the `xio_state`
layer forms a standard `xio` component as required by the SDK.
* **tls_adapter** The upper two layers hide all of the `xio` complexity from the `tls_adapter`
layer, so the bottom-layer `tls_adapter` layer is simple to implement.
 
## Two flavors of tlsio_adapter

A few TLS libraries have TLS handling integrated with TCP/IP 
functionality. Examples include Arduino, Apple OSX, and Apple iOS.

More commonly, TLS libraries are designed to work with Berkeley sockets
either directly or by wrapping socket functionality 
in a set of "bio" functions. Examples include Windows Schannel, WolfSSL, 
Mbed TLS, OpenSSL, and CycloneSSL. 

There are two flavors of the `tlsio_adapter` middle layer to handle these two different cases.
* The **tlsio_adapter_basic** is for TLS implementations that don't use sockets. It consists of
two files: `tlsio_adapter_basic.c`, and the shared file `tlsio_adapter_common.c`.
These files encapsulate the final `xio` callback, error handling, option handling, platform
adapter functions, read buffering, and validation of `xio_create` parameters.
* The **tlsio_adapter_with_sockets** does all the same work as `tlsio_adapter_basic`, 
and in addition it gives the low-level `tls_adapter` a 
pre-configured socket wrapped in a 
platform-independent `socket_async` component. The `tlsio_adapter_with_sockets`
does all the work of maintaining the socket, including creation, deletion, configuration,
opening, and closing, so the low-level `tls_adapter` can simply use the `socket_async`
as provided. It consists of two files: `tlsio_adapter_with_sockets.c` and 
the shared file `tlsio_adapter_common.c`.

## The low-level tls_adapter

### Common functions
Low-level `tls_adapter` components all implement the functions declared in
`tls_adapter_common.h`. This header file declares all of the `tls_adapter` 
functions except for object creation:

```c
typedef struct TLS_ADAPTER_INSTANCE_TAG* TLS_ADAPTER_INSTANCE_HANDLE;

// Called only once to initialize the underlying TLS API if needed.
// Returns 0 for success, and XIO_ASYNC_RESULT_FAILURE for error
int tls_adapter_common_init(void);

// Called only once to de-initialize the underlying TLS API if needed.
void tls_adapter_common_deinit(void);

// Called repeatedly until it returns either XIO_ASYNC_RESULT_SUCCESS
// or XIO_ASYNC_RESULT_FAILURE
XIO_ASYNC_RESULT tls_adapter_common_open(TLS_ADAPTER_INSTANCE_HANDLE adapter);

// Tls adapters are created with either tls_adapter_basic_create or 
// tls_adapter_with_sockets_create
void tls_adapter_common_close_and_destroy(TLS_ADAPTER_INSTANCE_HANDLE adapter);

// Return a bit-or of TLSIO_OPTION_BIT values to specify which options the tls
// adapter supports. For microcontrollers this will usually be either
// TLSIO_OPTION_BIT_TRUSTED_CERTS or TLSIO_OPTION_BIT_NONE
int tls_adapter_common_get_option_caps(void);

// buffer is guaranteed non-NULL and size_t is guaranteed smaller than INTMAX
// Return positive for success, 0 if waiting for data, and 
// XIO_ASYNC_RESULT_FAILURE for error
int tls_adapter_common_read(TLS_ADAPTER_INSTANCE_HANDLE adapter,
        uint8_t* buffer, size_t size);

// buffer is guaranteed non-NULL and size_t is guaranteed smaller than INTMAX
// Return positive number of bytes written for success, 0 if waiting, 
// and XIO_ASYNC_RESULT_FAILURE for error
int tls_adapter_common_write(TLS_ADAPTER_INSTANCE_HANDLE adapter,
        const uint8_t* buffer, size_t size);
```
### tls_adapter creation functions

To match the two flavors of the mid-level `tlsio_adapter`, there are two possible
ways to create a `tls_adapter`. If you've included `tlsio_adapter_with_sockets.c`
in your project, then you'll need to write your `tls_adapter` creation fuction to 
use the declaration in `tls_adapter_with_sockets.h`:
```c
TLS_ADAPTER_INSTANCE_HANDLE tls_adapter_with_sockets_create(TLSIO_OPTIONS* tlsio_options,
        const char* hostname, SOCKET_ASYNC_HANDLE socket_async);
```
If your TLS implementation doesn't want a `socket_async` opened for it, then 
you'll include `tlsio_adapter_basic.c` in your project instead of
`tlsio_adapter_with_sockets.c`, and you'll write your `tls_adapter` creation
function to use the declaration in `tls_adapter_basic.h`:
```c
TLS_ADAPTER_INSTANCE_HANDLE tls_adapter_basic_create(TLSIO_OPTIONS* tlsio_options,
        const char* hostname, uint16_t port);
```
As you can see, the only difference is that the "basic" variety gives you a port number,
while the "with_sockets" variety passes in a configured and open `socket_async`.

### Writing a tls_adapter
For more information on how to write your own `tls_adapter`, see
the [tls_adapter implementer's guide](tls_adapter_implementers_guide).