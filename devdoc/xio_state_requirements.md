# xio_state

## Overview

This specification defines the `xio_state` component, which provides state management and callback handling
for `xio` components. It works in conjunction with an `xio_adapter` component that implements
upstream and downstream `xio` commmunication.

The `xio_state` component does not support open/close/open sequences. Each instance of an `xio_state` and 
its associated adapter may be successfully opened only once. It would be possible to allow open/close/open
sequences, but the benefits would be trivial compared to the cost of designing, specifying, coding, and 
testing the feature.

## References

[xio_adapter_requirements](xio_adapter_requirements.md)

[xio.h](/inc/azure_c_shared_utility/xio.h)</br>
[xio_adapter.h](/inc/azure_c_shared_utility/xio_adapter.h)</br>
[xio_state.h](/inc/azure_c_shared_utility/xio_state.h)</br>



## Exposed API

The `xio_state` implements and export all the functions defined in `xio_state.h`. 
```c
CONCRETE_IO_HANDLE xio_state_create(
    const XIO_ADAPTER_INTERFACE* adapter_interface, 
    void* io_create_parameters);
    
// These functions replace the corresponding functions in your 
// specific xio_get_interface_description
void xio_state_destroy(CONCRETE_IO_HANDLE, xio);
int xio_state_open_async(CONCRETE_IO_HANDLE xio, ON_IO_OPEN_COMPLETE on_open_complete, 
    void* on_open_complete_context ON_BYTES_RECEIVED, on_bytes_received, void*, 
    on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
int xio_state_close_async(CONCRETE_IO_HANDLE xio, ON_IO_CLOSE_COMPLETE on_close_complete, 
    void* callback_context);
int xio_state_send_async(CONCRETE_IO_HANDLE xio, const void* buffer, size_t size, 
    ON_SEND_COMPLETE on_send_complete, void* callback_context);
void xio_state_dowork(CONCRETE_IO_HANDLE xio);
int xio_state_setoption(CONCRETE_IO_HANDLE xio, const char* optionName, const void* value);
OPTIONHANDLER_HANDLE xio_state_retrieveoptions(CONCRETE_IO_HANDLE xio);

```

The following types from `xio.h` are also referenced in individual requirements.
```c

typedef enum IO_OPEN_RESULT_TAG
{
    IO_OPEN_OK,
    IO_OPEN_ERROR,
    IO_OPEN_CANCELLED
} IO_OPEN_RESULT;

typedef enum IO_SEND_RESULT_TAG
{
    IO_SEND_OK,
    IO_SEND_ERROR,
    IO_SEND_CANCELLED
} IO_SEND_RESULT;

typedef void(*ON_BYTES_RECEIVED)(void* context, const unsigned char* buffer, size_t size);
typedef void(*ON_SEND_COMPLETE)(void* context, IO_SEND_RESULT send_result);
typedef void(*ON_IO_OPEN_COMPLETE)(void* context, IO_OPEN_RESULT open_result);
typedef void(*ON_IO_CLOSE_COMPLETE)(void* context);
typedef void(*ON_IO_ERROR)(void* context);
```
## Observable State
The observable state of the `xio_state` component is determined by which of the component's interface 
functions have been called and which callbacks have been performed. The components's internal state
is not directly visible from the outside, but it maps directly to the external state. 
The observable states are defined as follows:

* XIO_STATE_INITIAL means that the `xio_state` component is newly created and `xio_state_open_async`
  has not been called.
* XIO_STATE_OPENING means that the `xio_state_open_async` call has completed successfully 
    but the `on_open_complete` callback has not yet been performed.
* XIO_STATE_OPEN means that the `on_open_complete` callback has returned with `IO_OPEN_OK`.
* XIO_STATE_CLOSING means that either the `xio_state_close_async` call has completed 
  successfully but the `on_close_complete` callback has not yet been performed, or that the 
  `on_open_complete` callback has returned with `IO_OPEN_CANCELLED`.
* XIO_STATE_CLOSED means that the callback provided to `xio_state_close_async` has been received.
* XIO_STATE_ERROR means either
  *  `on_io_error` has been called from `xio_state_dowork`, or
  *  the callback provided to `xio_state_open_async` has returned `IO_OPEN_ERROR`.

## State Transitions
This list shows the effect of the calls as a function of state with happy internal 
functionality. Unhappy functionality is not shown. The `xio_state_setoption` and 
`xio_state_retrieveoptions` calls are not shown because they have no effect on 
state and are always allowed. Calls to `xio_state_send_async` also do not affect 
state, and are allowed only during XIO_STATE_OPEN.

All transitions into XIO_STATE_CLOSED are understood to pass through XIO_STATE_CLOSING.


<table>
  <tr>From state <b>XIO_STATE_INITIAL</b></tr>
  <tr>
    <td>xio_state_destroy</td>
    <td>ok (destroyed)</td>
  </tr>
  <tr>
    <td>xio_state_open_async</td>
    <td>ok, enter XIO_STATE_OPENING</td>
  </tr>
  <tr>
    <td>xio_state_close_async</td>
    <td>fail, log error, remain in XIO_STATE_INITIAL (usage error)</td>
  </tr>
  <tr>
    <td>xio_state_dowork</td>
    <td>ok (does nothing), remain in XIO_STATE_INITIAL</td>
  </tr>
</table>

<table>
  <tr>From state <b>XIO_STATE_OPENING</b></tr>
  <tr>
    <td>xio_state_destroy</td>
    <td>log error, force immediate close, destroy (destroyed)</td>
  </tr>
  <tr>
    <td>xio_state_open_async</td>
    <td>fail, log error, remain in XIO_STATE_OPENING (usage error)</td>
  </tr>
  <tr>
    <td>xio_state_close_async</td>
    <td>ok, enter XIO_STATE_CLOSING</td>
  </tr>
  <tr>
    <td>xio_state_dowork</td>
    <td>ok (continue opening), remain in XIO_STATE_OPENING or enter XIO_STATE_OPEN</td>
  </tr>
</table>

<table>
  <tr>From state <b>XIO_STATE_OPEN</b></tr>
  <tr>
    <td>xio_state_destroy</td>
    <td>(a possible normal SDK behavior) force immediate close, destroy (destroyed)</td>
  </tr>
  <tr>
    <td>xio_state_open_async</td>
    <td>fail, log error, remain in XIO_STATE_OPEN (usage error)</td>
  </tr>
  <tr>
    <td>xio_state_close_async</td>
    <td>ok, enter XIO_STATE_CLOSING</br>
        (xio_adapters without internal async close will</br>
        then immediately enter XIO_STATE_CLOSED)</td>
  </tr>
  <tr>
    <td>xio_state_dowork</td>
    <td>ok (send and receive as necessary), remain in XIO_STATE_OPEN</td>
  </tr>
</table>

Upper layers may issue `xio_close` commands in response to failed `on_send_complete` callbacks
while `xio_state` is in `XIO_STATE_CLOSING`. The `xio_state` component will remain 
in `XIO_STATE_CLOSING` during these extra `xio_close` calls. An `xio_state` component that
gets stuck too long in `XIO_STATE_CLOSING` may be killed with `xio_state_destroy`.

<table>
  <tr>From state <b>XIO_STATE_CLOSING</b></tr>
  <tr>
    <td>xio_state_destroy</td>
    <td>log error, force immediate close, destroy (destroyed)</td>
  </tr>
  <tr>
    <td>xio_state_open_async</td>
    <td>fail, log error, remain in XIO_STATE_CLOSING (usage error)</td>
  </tr>
  <tr>
    <td>xio_state_close_async</td>
    <td>ok, do nothing and remain in XIO_STATE_CLOSING</td>
  </tr>
  <tr>
    <td>xio_state_dowork</td>
    <td>ok (continue graceful closing) , remain in XIO_STATE_CLOSING or <br/>enter XIO_STATE_CLOSED</td>
  </tr>
</table>

<table>
  <tr>From state <b>XIO_STATE_CLOSED</b></tr>
  <tr>
    <td>xio_state_destroy</td>
    <td>ok (destroyed)</td>
  </tr>
  <tr>
    <td>xio_state_open_async</td>
    <td>fail, log error, remain in XIO_STATE_CLOSED (usage error)</td>
  </tr>
  <tr>
    <td>xio_state_close_async</td>
    <td>ok, remain in XIO_STATE_CLOSED</td>
  </tr>
  <tr>
    <td>xio_state_dowork</td>
    <td>ok (does nothing), remain in XIO_STATE_CLOSED</td>
  </tr>
</table>

<table>
  <tr>From state <b>XIO_STATE_ERROR</b></tr>
  <tr>
    <td>xio_state_destroy</td>
    <td>force immediate close, destroy (destroyed)</td>
  </tr>
  <tr>
    <td>xio_state_open_async</td>
    <td>fail, log error, remain in XIO_STATE_ERROR (usage error)</td>
  </tr>
  <tr>
    <td>xio_state_close_async</td>
    <td>ok, enter XIO_STATE_CLOSING</br>
        (xio_adapters without internal async close will</br>
        then immediately enter XIO_STATE_CLOSED)</td>
  </tr>
  <tr>
    <td>xio_state_dowork</td>
    <td>ok (does nothing), remain in XIO_STATE_ERROR</td>
  </tr>
</table>

![State transition diagram](img/xio_state_diagram.png)

## Definitions 

### State Definitions

#### Explicit state transitions

Throughout this document, state transitions only occur as explicitly specified. If no state 
transition is called out, then none is allowed. (So "do nothing" is always understood as the default.)

#### Specified state transitions

Requirements in this document use the phrase "shall enter XIO_STATE_XXXX" to specify behavior. 
Here are the definitions of the state transition phrases:

##### "enter XIO_STATE_ERROR"
**SRS_XIO_STATE_30_005: [** The phrase "enter XIO_STATE_ERROR" means `xio_state` shall call the 
`on_io_error` function and pass the `on_io_error_context` that was supplied in 
`xio_state_open_async`. **]**

##### "enter XIO_STATE_CLOSING"
**SRS_XIO_STATE_30_006: [** The phrase "enter XIO_STATE_CLOSING" means `xio_state` shall 
call `xio_adapter_close` during subsequent calls to `xio_state_dowork`. **]**

##### "enter XIO_STATE_CLOSED"
**SRS_XIO_STATE_30_009: [** The phrase "enter XIO_STATE_CLOSED" means `xio_state` shall 
deuque any unsent messages per [Message Processing Requirements](#message-processing-requirements), 
then call the `on_close_complete` function and pass the `on_close_complete_context` 
that was supplied in `xio_state_close_async`. **]**

##### "enter XIO_STATE_OPENING"
The phrase "enter XIO_STATE_OPENING" means `xio_state` will continute calling `xio_adapter_open` 
during subsequent `xio_state_dowork` calls, but no other externally visible action is 
being specified.

##### "enter XIO_STATE_OPEN"
**SRS_XIO_STATE_30_007: [** The phrase "enter XIO_STATE_OPEN" means `xio_state` shall 
call the `on_open_complete` function and pass IO_OPEN_OK and the 
`on_open_complete_context` that was supplied in `xio_state_open_async`. **]**

## Message Processing Requirements

**SRS_XIO_STATE_30_040: [** When `xio_state` enqueues a message it shall make a 
copy of the data supplied in `xio_state_send_async`. **]**

**SRS_XIO_STATE_30_044: [** If a message was sent successfully, then after it is dequeued
`xio_state` shall call the message's `on_send_complete` along with its associated 
`callback_context` and `IO_SEND_OK`. **]**

**SRS_XIO_STATE_30_045: [** If a message was not sent successfully, then after it is dequeued
`xio_state` shall call the message's `on_send_complete` along with its associated 
`callback_context` and `IO_SEND_ERROR`. **]**

**SRS_XIO_STATE_30_047: [** When `xio_state` dequeues a message it shall free the message's data. **]**

## API Calls


###   xio_state_create
A specialized create function, `xio_state_create` is not used directly in interface descriptions.

```c

CONCRETE_IO_HANDLE xio_state_create(
    const XIO_ADAPTER_INTERFACE* adapter_interface, 
    void* io_create_parameters);

```

**SRS_XIO_STATE_30_013: [** If the `adapter_interface` parameter is NULL, 
`xio_state_create` shall log an error and return NULL. **]**

**SRS_XIO_STATE_30_010: [** The `xio_state_create` shall allocate and initialize 
all necessary resources, call `xio_adapter_create`, and return an instance of the `xio_state` in XIO_STATE_INITIAL. **]**

**SRS_XIO_STATE_30_011: [** If `xio_adapter_create` fails or any resource allocation 
fails, `xio_state_create` shall return NULL. **]**

**SRS_XIO_STATE_30_012: [** The `xio_state_create` shall pass 
`io_create_parameters` to `xio_adapter_create`. **]**


###   xio_state_destroy

Closes the `xio_adapter` connection and releases all resources.

```c
void xio_state_destroy(CONCRETE_IO_HANDLE xio_state_handle);
```

**SRS_XIO_STATE_30_020: [** If `xio_state_handle` is NULL, `xio_state_destroy` shall do nothing. **]**

**SRS_XIO_STATE_30_022: [** If `xio_state` is in XIO_STATE_OPENING,
    `xio_state_destroy` shall call `xio_adapter_close` then
    [enter XIO_STATE_CLOSED](#enter-XIO_STATE_CLOSED "Deque each unsent per requirements, 
    then call the `on_close_complete` function and pass the `on_close_complete_context` 
    that was supplied in `xio_state_close_async`")
    before releasing resources. **]**

**SRS_XIO_STATE_30_023: [** If `xio_state` is in XIO_STATE_OPEN,
    `xio_state_destroy` shall call `xio_adapter_close` then
    [enter XIO_STATE_CLOSED](#enter-XIO_STATE_CLOSED "Deque each unsent per requirements, 
    then call the `on_close_complete` function and pass the `on_close_complete_context` 
    that was supplied in `xio_state_close_async`")
    before releasing resources. **]**

**SRS_XIO_STATE_30_024: [** If `xio_state` is in XIO_STATE_CLOSING,
    `xio_state_destroy` shall call `xio_adapter_close` then
    [enter XIO_STATE_CLOSED](#enter-XIO_STATE_CLOSED "Deque each unsent per requirements, 
    then call the `on_close_complete` function and pass the `on_close_complete_context` 
    that was supplied in `xio_state_close_async`")
    before releasing resources. **]**

**SRS_XIO_STATE_30_025: [** If `xio_state` is in XIO_STATE_ERROR,
    `xio_state_destroy` shall call `xio_adapter_close` then
    [enter XIO_STATE_CLOSED](#enter-XIO_STATE_CLOSED "Deque each unsent per requirements, 
    then call the `on_close_complete` function and pass the `on_close_complete_context` 
    that was supplied in `xio_state_close_async`")
    before releasing resources. **]**

**SRS_XIO_STATE_30_021: [** The `xio_state_destroy` call shall release all allocated 
resources, call `xio_adapter_destroy`, and then release `xio_state_handle`. **]**


###   xio_state_open_async
Begins the process of opening the `xio_adapter` connection.

The `xio_state_open_async` function will need to store the provided `on_bytes_received`, 
`on_bytes_received_context`, `on_io_error`, `on_io_error_context`, 
`on_open_complete`,  and `on_open_complete_context` parameters for later use.
The use of these stored parameters is specified in other sections of this document.

```c
int xio_state_open_async(
    CONCRETE_IO_HANDLE xio_state_handle,
    ON_IO_OPEN_COMPLETE on_open_complete,
    void* on_open_complete_context,
    ON_BYTES_RECEIVED on_bytes_received,
    void* on_bytes_received_context,
    ON_IO_ERROR on_io_error,
    void* on_io_error_context);
```

**SRS_XIO_STATE_30_030: [** If any of the `xio_state_handle`, `on_open_complete`, 
`on_bytes_received`, or `on_io_error` parameters is NULL, `xio_state_open_async` shall log an error and return `_FAILURE_`. **]**

**SRS_XIO_STATE_30_037: [** If `xio_state` is in any state other than 
XIO_STATE_INITIAL when `xio_state_open_async` is called, it shall log an error, 
and return `_FAILURE_`. **]**

**SRS_XIO_STATE_30_035: [** On success, xio_state_open_async shall cause the adapter to 
[enter XIO_STATE_OPENING](#enter-XIO_STATE_OPENING "`xio_state` will continute calling `xio_adapter_open` 
during subsequent `xio_state_dowork` calls, but no other externally visible action is 
being specified.")
and return 0. **]**

**SRS_XIO_STATE_30_039: [** On failure, `xio_state_open_async` shall not call `on_open_complete`. **]**


###   xio_state_close_async

This function begins the process of closing the `xio_adapter` connection and making appropriate callbacks.

```c
int xio_state_close_async(CONCRETE_IO_HANDLE xio_state_handle, 
    ON_IO_CLOSE_COMPLETE on_close_complete, void* callback_context);
```

**SRS_XIO_STATE_30_050: [** If the `xio_state_handle` or `on_close_complete` 
parameter is NULL, `xio_state_close_async` shall log an error and return `XIO_ASYNC_RESULT_FAILURE`. **]**

**SRS_XIO_STATE_30_054: [** On failure, `xio_state_close_async` shall not 
call `on_close_complete`. **]**

**SRS_XIO_STATE_30_053: [** If `xio_state` is in XIO_STATE_INITIAL `xio_state_close_async` 
shall log an error and return `XIO_ASYNC_RESULT_FAILURE`. **]**

**SRS_XIO_STATE_30_055: [** If `xio_state` is in XIO_STATE_CLOSING then 
`xio_state_close_async` shall do nothing and return 0. **]**

**SRS_XIO_STATE_30_059: [** If `xio_state` is in XIO_STATE_CLOSED then 
`xio_state_close_async` shall do nothing and return 0. **]**

**SRS_XIO_STATE_30_057: [** If `xio_state` is in XIO_STATE_OPENING then `xio_state_close_async` 
shall shall call `on_open_complete` with the `on_open_complete_context` supplied in 
`xio_state_open_async` and `IO_OPEN_CANCELLED`. **]**

**SRS_XIO_STATE_30_058: [** `xio_state_close_async` shall call `xio_adapter_close` on its adapter. **]**

**SRS_XIO_STATE_30_056: [** If `xio_adapter_close` returns `XIO_ASYNC_RESULT_WAITING`, 
    `xio_state_close_async` shall 
    [enter XIO_STATE_CLOSING](#enter-XIO_STATE_CLOSING "Call `xio_adapter_close` during 
    subsequent calls to `xio_state_dowork`")
    and return 0. **]**

**SRS_XIO_STATE_30_051: [** If `xio_adapter_close` returns `XIO_ASYNC_RESULT_SUCCESS`, 
    `xio_state_close_async` shall 
    [enter XIO_STATE_CLOSED](#enter-XIO_STATE_CLOSED "Deque each unsent per requirements, 
    then call the `on_close_complete` function and pass the `on_close_complete_context` 
    that was supplied in `xio_state_close_async`")
    and return 0. **]**

**SRS_XIO_STATE_30_052: [** If `xio_adapter_close` returns
    `XIO_ASYNC_RESULT_FAILURE`, `xio_state_close_async` shall 
    [enter XIO_STATE_CLOSED](#enter-XIO_STATE_CLOSED "Deque each unsent per requirements, 
    then call the `on_close_complete` function and pass the `on_close_complete_context` 
    that was supplied in `xio_state_close_async`")
    and return 0. **]**


###   xio_state_send_async

Queue up a message for transmission, then immediately attempt to send the first message 
in the queue.

```c
int xio_state_send_async(CONCRETE_IO_HANDLE xio_state_handle, const void* buffer, 
    size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
```

**SRS_XIO_STATE_30_060: [** If any of the `xio_state_handle`, `buffer`, or `on_send_complete` 
parameters is NULL, `xio_state_send_async` shall log an error and return 
`XIO_ASYNC_RESULT_FAILURE`. **]**

**SRS_XIO_STATE_30_067: [** If the `size` is 0, `xio_state_send_async` shall log an 
error and return `XIO_ASYNC_RESULT_FAILURE`. **]**

**SRS_XIO_STATE_30_065: [** If `xio_state` state is not XIO_STATE_OPEN, 
`xio_state_send_async` shall log an error and return `XIO_ASYNC_RESULT_FAILURE`. **]**

**SRS_XIO_STATE_30_066: [** On failure, `on_send_complete` shall not be called. **]**

**SRS_XIO_STATE_30_064: [** If the supplied message cannot be enqueued for transmission, 
`xio_state_send_async` shall log an error and return `XIO_ASYNC_RESULT_FAILURE`. **]**

**SRS_XIO_STATE_30_063: [** On success, `xio_state_send_async` shall enqueue for 
transmission the `on_send_complete`, the `callback_context`, the `size`, and the 
`buffer` per [Message Processing Requirements](#message-processing-requirements)
and then return 0. **]**

**SRS_XIO_STATE_30_061: [** On success, after enqueuing the message, `xio_state_send_async` 
shall invoke the Data Transmission behavior of `xio_state_dowork`. **]**


###   xio_state_dowork
The `xio_state_dowork` call executes async jobs for the `xio_state`. This includes 
connection completion, sending to the underlying connection, and checking the 
underlying connection for available bytes to read.
```c
void xio_state_dowork(CONCRETE_IO_HANDLE xio_state_handle);
```

**SRS_XIO_STATE_30_070: [** If the `xio_state_handle` parameter is NULL, 
`xio_state_dowork` shall do nothing except log an error. **]**


#### Behavior selection

**SRS_XIO_STATE_30_072: [** If `xio_state` is in XIO_STATE_INITIAL then 
`xio_state_dowork` shall do nothing. **]**

**SRS_XIO_STATE_30_071: [** If `xio_state` is in XIO_STATE_ERROR then 
`xio_state_dowork` shall do nothing. **]**

**SRS_XIO_STATE_30_075: [** If `xio_state` is in XIO_STATE_CLOSED then 
`xio_state_dowork` shall do nothing. **]**

**SRS_XIO_STATE_30_076: [** If `xio_state` is in XIO_STATE_OPENING then 
`xio_state_dowork` shall perform only the 
[XIO_STATE_OPENING behaviors](#XIO_STATE_OPENING-behaviors). **]**

**SRS_XIO_STATE_30_077: [** If `xio_state` is in XIO_STATE_OPEN  then 
`xio_state_dowork` shall perform only the 
[Data transmission behaviors](#data-transmission-behaviors) and 
the [Data reception behaviors](#data-reception-behaviors). **]**

**SRS_XIO_STATE_30_078: [** If `xio_state` is in XIO_STATE_CLOSING then 
`xio_state_dowork` shall perform only the 
[XIO_STATE_CLOSING behaviors](#XIO_STATE_CLOSING-behaviors). **]**

#### XIO_STATE_OPENING behaviors

Transitioning from XIO_STATE_OPENING to XIO_STATE_OPEN may require multiple calls 
to `xio_state_dowork`. The number of calls required is not specified.

**SRS_XIO_STATE_30_080: [** The `xio_state_dowork` shall call `xio_adapter_open`
on the underlying `xio_adapter`. **]**

**SRS_XIO_STATE_30_081: [** If the `xio_adapter` returns `XIO_ASYNC_RESULT_WAITING`, 
`xio_state_dowork` shall remain in the XIO_STATE_OPENING state.  **]**

**SRS_XIO_STATE_30_082: [** If the `xio_adapter` returns `XIO_ASYNC_RESULT_FAILURE`, 
`xio_state_dowork`  shall log an error, call `on_open_complete` with the 
`on_open_complete_context` parameter provided in `xio_state_open_async` and `IO_OPEN_ERROR`, and 
[enter XIO_STATE_CLOSED](#enter-XIO_STATE_CLOSED "Deque each unsent per requirements, 
then call the `on_close_complete` function and pass the `on_close_complete_context` 
that was supplied in `xio_state_close_async`")
. **]**

**SRS_XIO_STATE_30_083: [** If `xio_adapter` returns `XIO_ASYNC_RESULT_SUCCESS`, 
`xio_state_dowork` shall 
[enter XIO_STATE_OPEN](#enter-XIO_STATE_OPEN "Call the `on_open_complete` 
function and pass IO_OPEN_OK and the `on_open_complete_context` 
that was supplied in `xio_state_open_async`."). **]**

#### Data transmission behaviors

**SRS_XIO_STATE_30_096: [** If there are no enqueued messages available, 
`xio_state_dowork` shall do nothing. **]**

**SRS_XIO_STATE_30_090: [** If there are any unsent messages in the queue, `xio_state_dowork` 
shall call `xio_adapter_write` on the `xio_adapter` and pass in the first message in the queue. **]**

**SRS_XIO_STATE_30_091: [** If `xio_adapter_write` is able to send all the bytes in the 
enqueued message, `xio_state_dowork` shall deque the message per
[Message Processing Requirements](#message-processing-requirements). **]**

**SRS_XIO_STATE_30_093: [** If `xio_adapter_write` was not able to send an entire enqueued 
message at once, subsequent calls to `xio_state_dowork` shall continue to send the remaining bytes. **]**

**SRS_XIO_STATE_30_095: [** If the send process fails before sending all of the bytes in an 
enqueued message, `xio_state_dowork` shall deque the message per
[Message Processing Requirements](#message-processing-requirements) 
and 
[enter XIO_STATE_ERROR](#enter-XIO_STATE_ERROR "Call the `on_io_error` function and pass 
the `on_io_error_context` that was supplied in `xio_state_open_async`."). 
**]**


#### Data reception behaviors

The `xio_adapter` uses the `on_bytes_received` callback supplied during `xio_state_open_async` to 
report the bytes that it receives, so data handling details are specified in the 
[xio_adapter_requirements](xio_adapter_requirements.md) document.

**SRS_XIO_STATE_30_100: [** The `xio_state_dowork` shall repeatedly call `xio_adapter_read` 
on the underlying `xio_adapter` until the return value is not `XIO_ASYNC_RESULT_SUCCESS`. **]**

**SRS_XIO_STATE_30_103: [** If the `xio_adapter_read` returns acquires data, it shall 
report it with the `on_received` callback and `on_received_context` provided in 
`xio_state_open`. **]**

**SRS_XIO_STATE_30_101: [** If the `xio_adapter` returns 
`XIO_ASYNC_RESULT_WAITING`, `xio_state_dowork` shall do nothing. **]**

**SRS_XIO_STATE_30_102: [** If the `xio_adapter` returns `XIO_ASYNC_RESULT_FAILURE` 
then `xio_state_dowork` shall [enter XIO_STATE_ERROR](#enter-XIO_STATE_ERROR "Call the `on_io_error` 
function and pass the `on_io_error_context` that was supplied in `xio_state_open_async`."). **]**

#### XIO_STATE_CLOSING behaviors

**SRS_XIO_STATE_30_105: [** `xio_state_dowork` shall call `xio_adapter_close` on 
the `xio_adapter` provided during `xio_state_open_async`. **]**

**SRS_XIO_STATE_30_106: [** If `xio_adapter_close` returns `XIO_ASYNC_RESULT_FAILURE`, 
`xio_state_dowork` shall log an error and 
[enter XIO_STATE_CLOSED](#enter-XIO_STATE_CLOSED "Deque each unsent per requirements, then 
call the `on_close_complete` function and pass the `on_close_complete_context` that was 
supplied in `xio_state_close_async`")
. **]**

**SRS_XIO_STATE_30_107: [** If `xio_adapter_close` returns `XIO_ASYNC_RESULT_SUCCESS`, 
`xio_state_dowork` shall 
[enter XIO_STATE_CLOSED](#enter-XIO_STATE_CLOSED "Deque each unsent per requirements, 
then call the `on_close_complete` function and pass the `on_close_complete_context` 
that was supplied in `xio_state_close_async`")
. **]**

**SRS_XIO_STATE_30_108: [** If the `xio_adapter_close` returns 
`XIO_ASYNC_RESULT_WAITING`, `xio_state_dowork` shall remain in the XIO_STATE_CLOSING state. **]**

###   xio_state_setoption
The `xio_state` component delegates the `xio_state_setoption` call to the `xio_adapter`
that was supplied during `xio_state_create`. 

```c
int xio_state_setoption(CONCRETE_IO_HANDLE xio_state_handle, 
    const char* optionName, const void* value);
```
**SRS_XIO_STATE_30_120: [** If any of the the `xio_state_handle`, `optionName`, or `value` 
parameters is NULL, `xio_state_setoption` shall do nothing except log an error and 
return `_FAILURE_`. **]**

**SRS_XIO_STATE_30_121 [** `xio_state` shall delegate the behavior of `xio_state_setoption` 
to the `xio_adapter` supplied in `xio_state_create`. **]**


###   xio_state_retrieveoptions
The `xio_state` component delegates the `xio_state_retrieveoptions` call to the `xio_adapter`
that was supplied during xio_state_create`. 

```c
OPTIONHANDLER_HANDLE xio_state_retrieveoptions(CONCRETE_IO_HANDLE xio_state_handle);
```

**SRS_XIO_STATE_30_160: [** If the `xio_state_handle` parameter is NULL, 
`xio_state_retrieveoptions` shall do nothing except log an error and return `_FAILURE_`. **]**

**SRS_XIO_STATE_30_161: [** `xio_state` shall delegate the behavior of 
`xio_state_retrieveoptions` to the `xio_adapter` supplied during 
`xio_state_create`. **]**
