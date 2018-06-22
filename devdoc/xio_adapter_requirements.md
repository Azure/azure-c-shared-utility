# xio_adapter

## Overview

This specification defines the `xio_adapter` component, which provides stateless message handling
for `xio` components. It works in conjunction with an `xio_state` component that implements
state handling and callbacks.

**Important:** The `xio_adapter` component is only used by `xio_state`, which performs all state 
management. Therefore an `xio_adapter` component should never perform any of its own 
`xio` state management. For example, there is no need to ensure that `close` has been
called before performing `destroy`.

There is no abstract `xio_adapter` component. 

## References

[xio_state_requirements](xio_state_requirements.md)

[xio.h](/inc/azure_c_shared_utility/xio.h)</br>
[xio_async_result.h](/inc/azure_c_shared_utility/xio_async_result.h)</br>
[xio_adapter.h](/inc/azure_c_shared_utility/xio_adapter.h)</br>
[xio_state.h](/inc/azure_c_shared_utility/xio_state.h)</br>

## Parameter Checking

`xio_adapter` components are designed to be called directly only from an `xio_state` component,
and the `xio_state` component performs parameter checking before making calls into the 
`xio_adapter`. For that reason, this specification intentionally omits any requirement for 
parameter checking at the `xio_adapter` surface. It is up to the implementer to choose 
whether their implementation checks input parameters.

## Exposed API

 `xio_adapter` uses return values specified in defined in [xio_async_result.h](/inc/azure_c_shared_utility/xio_async_result.h). 
```c
typedef enum XIO_ASYNC_RESULT_TAG
{
    XIO_ASYNC_RESULT_SUCCESS = 1
    XIO_ASYNC_RESULT_WAITING = 0,
    XIO_ASYNC_RESULT_FAILURE = -1,
} XIO_ASYNC_RESULT;
```
 Each `xio_adapter` implements and export all the functions defined in [xio_adapter.h](/inc/azure_c_shared_utility/xio_adapter.h). 
```c
typedef struct XIO_ADAPTER_INTERFACE_TAG* XIO_ADAPTER_INTERFACE_HANDLE;
typedef struct XIO_ADAPTER_INSTANCE_TAG* XIO_ADAPTER_INSTANCE_HANDLE;


typedef XIO_ADAPTER_INSTANCE_HANDLE(*XIO_ADAPTER_CREATE)(void);
typedef void(*XIO_ADAPTER_DESTROY)(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance);
typedef XIO_ASYNC_RESULT(*XIO_ADAPTER_OPEN)(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance, 
    XIO_ADAPTER_CONFIG_HANDLE xio_adapter_config, 
    ON_BYTES_RECEIVED on_received, void* on_received_context);
typedef XIO_ASYNC_RESULT(*XIO_ADAPTER_CLOSE)(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance);
typedef XIO_ASYNC_RESULT(*XIO_ADAPTER_READ)(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance);
typedef int(*XIO_ADAPTER_WRITE)(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance, 
    const uint8_t* buffer, uint32_t count);


// Declarations for mocking only -- actual implementations are exposed only
// within an XIO_ADAPTER_CONFIG_INTERFACE
XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_create(void);
void xio_adapter_destroy(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter);
XIO_ASYNC_RESULT xio_adapter_open(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance, 
    XIO_ADAPTER_CONFIG_HANDLE, xio_adapter_config,
    ON_BYTES_RECEIVED on_received, void* on_received_context);
XIO_ASYNC_RESULT xio_adapter_close(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance);
XIO_ASYNC_RESULT xio_adapter_read(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance);
// write returns 0 for waiting, non-zero for data transferred, 
// or XIO_ASYNC_RESULT_FAILURE
int xio_adapter_write(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance, 
    const uint8_t* buffer, uint32_t buffer_size);

// XIO_ADAPTER_INTERFACE is always acquired through a concrete type, so there's no need
// for a generic getter function.
typedef struct XIO_ADAPTER_INTERFACE_TAG
{
    XIO_ADAPTER_CREATE create;
    XIO_ADAPTER_DESTROY destroy;
    XIO_ADAPTER_OPEN open;
    XIO_ADAPTER_CLOSE close;
    XIO_ADAPTER_READ read;
    XIO_ADAPTER_WRITE write;
} XIO_ADAPTER_INTERFACE;

```
**]**

## API Calls

Do not use the function names in the API in your concrete implementation because they that would
have the potential for creating name collisions. Instead, use an appropriate static
function and expose it within an `XIO_ADAPTER_INTERFACE`.

###   xio_adapter_create

```c
XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_create(void);
```
**SRS_XIO_ADAPTER_30_000: [** The `xio_adapter_create` shall allocate and initialize all 
necessary resources and return an instance of the `xio_adapter`. **]**

**SRS_XIO_ADAPTER_30_001: [** If any resource allocation fails, `xio_adapter_create` shall 
log an error and return NULL. **]**


###   xio_adapter_destroy

```c
void xio_adapter_destroy(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter);
```

**SRS_XIO_ADAPTER_30_010: [** The `xio_adapter_destroy` shall release all of the 
`xio_adapter` resources. **]**

###   xio_adapter_open

The `xio_adapter_open` function does whatever is necessary to prepare to 
accept read and write calls.

This function will need to store the 
`on_received` and `on_received_context` for later use as specifed in `xio_adapter_receive`.

```c
XIO_ASYNC_RESULT xio_adapter_open(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance, 
    XIO_ADAPTER_CONFIG_HANDLE, xio_adapter_config,
    ON_BYTES_RECEIVED on_received, void* on_received_context);
```

**SRS_XIO_ADAPTER_30_020: [** On success, `xio_adapter_open` shall return 
`XIO_ASYNC_RESULT_SUCCESS`. **]**

**SRS_XIO_ADAPTER_30_021: [** On failure, `xio_adapter_open` shall log an error and 
return `XIO_ASYNC_RESULT_FAILURE`. **]**

**SRS_XIO_ADAPTER_30_022: [** If `xio_adapter_open` needs to be called again to 
complete the opening process, it shall return `XIO_ASYNC_RESULT_WAITING`. **]**

###   xio_adapter_close

The `xio_adapter_close` function does whatever is necessary to close down its operation.

This function is guaranteed to be called at least once before `xio_adapter_destroy`
is called, but it is not guaranteed to be called more than once. This is sufficient for
adapters that never return `XIO_ASYNC_RESULT_WAITING` from `xio_adapter_close`, but
adapters that may return `XIO_ASYNC_RESULT_WAITING` from `xio_adapter_close` may 
need to force closure when `xio_adapter_destroy` is called.

The `xio_adapter_instance` parameter is guaranteed to be non-NULL by the calling `xio-state`.

```c
XIO_ASYNC_RESULT xio_adapter_close(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance);
```

**SRS_XIO_ADAPTER_30_030: [** If `xio_adapter_close` needs to be called again to 
complete the shutdown process, it shall return `XIO_ASYNC_RESULT_WAITING`. **]**

**SRS_XIO_ADAPTER_30_031: [** On completion of the closure process, `xio_adapter_close` shall return 
`XIO_ASYNC_RESULT_SUCCESS`. **]**</br>
Note: The `xio_adapter_close` call never returns failure.


###   xio_adapter_read

The owning `xio_state` component will call `xio_adapter_read` repeatedly as long as data is 
available, so there is no need for this call to loop internally.

The `xio_adapter_instance` parameter is guaranteed to be non-NULL by the calling `xio-state`.

```c
XIO_ASYNC_RESULT xio_adapter_read(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance);
```
**SRS_XIO_ADAPTER_30_040: [** The `xio_adapter_read` function shall attempt to read from its 
underlying data source. **]**

**SRS_XIO_ADAPTER_30_041: [** If data is available from the underlying
data source, `xio_adapter_read` shall report 
the results using the stored `on_received` and `on_received_context` and return 
`XIO_ASYNC_RESULT_SUCCESS`. **]**

**SRS_XIO_ADAPTER_30_042: [** On failure, `xio_adapter_read` shall log an error 
and return `XIO_ASYNC_RESULT_FAILURE`. **]**

**SRS_XIO_ADAPTER_30_043: [** If no data is available from the
underlying data source, `xio_adapter_read` shall return `XIO_ASYNC_RESULT_WAITING`. **]**

If a remote host closes a connection, sockets usually report this as "end of file", 
which is not usually considered an error. However, it is a communications error 
in the context of the Azure IoT C SDK.

**SRS_XIO_ADAPTER_30_044: [** When applicable, the `xio_adapter_read` shall test for an 
"end of file" condition and treat it as an error. **]**

###   xio_adapter_write

The pointer parameters are guaranteed to be non-NULL 
by the calling `xio_state`, and `buffer_size` is guaranteed positive.

```c
int xio_adapter_write(XIO_ADAPTER_INSTANCE_HANDLE xio_adapter_instance, 
    const uint8_t* buffer, uint32_t buffer_size);
```

**SRS_XIO_ADAPTER_30_051: [** The `xio_adapter_write` shall attempt to write 
`buffer_size` characters from `buffer` to its underlying data sink. **]**

**SRS_XIO_ADAPTER_30_052: [** On success, `xio_adapter_write` shall return the 
number of characters from `buffer` that are written to the underlying data sink. **]**

**SRS_XIO_ADAPTER_30_053: [** On failure, `xio_adapter_write` shall log an error 
and return `XIO_ASYNC_RESULT_FAILURE`. **]**

**SRS_XIO_ADAPTER_30_054: [** If the underlying data sink is temporarily unable to 
accept data, `xio_adapter_write` shall return `XIO_ASYNC_RESULT_WAITING`. **]**
