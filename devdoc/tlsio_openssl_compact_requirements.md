tlsio_openssl_compact
=============

## Overview

tlsio_openssl_compact implements a tlsio adapter for compact OpenSSL platforms.  


## References

[TLS Protocol (RFC2246)](https://www.ietf.org/rfc/rfc2246.txt)

[TLS Protocol (generic information)](https://en.wikipedia.org/wiki/Transport_Layer_Security)

[OpenSSL](https://www.openssl.org/)


## Exposed API

**SRS_TLSIO_OPENSSL_COMPACT_30_001: [** The tlsio_openssl_compact shall implement and export all the Concrete functions in the VTable IO_INTERFACE_DESCRIPTION defined in the `xio.h`.
```c
typedef OPTIONHANDLER_HANDLE (*IO_RETRIEVEOPTIONS)(CONCRETE_IO_HANDLE concrete_io);
typedef CONCRETE_IO_HANDLE(*IO_CREATE)(void* io_create_parameters);
typedef void(*IO_DESTROY)(CONCRETE_IO_HANDLE concrete_io);
typedef int(*IO_OPEN)(CONCRETE_IO_HANDLE concrete_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context);
typedef int(*IO_CLOSE)(CONCRETE_IO_HANDLE concrete_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
typedef int(*IO_SEND)(CONCRETE_IO_HANDLE concrete_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
typedef void(*IO_DOWORK)(CONCRETE_IO_HANDLE concrete_io);
typedef int(*IO_SETOPTION)(CONCRETE_IO_HANDLE concrete_io, const char* optionName, const void* value);


typedef struct IO_INTERFACE_DESCRIPTION_TAG
{
    IO_RETRIEVEOPTIONS concrete_io_retrieveoptions;
    IO_CREATE concrete_io_create;
    IO_DESTROY concrete_io_destroy;
    IO_OPEN concrete_io_open;
    IO_CLOSE concrete_io_close;
    IO_SEND concrete_io_send;
    IO_DOWORK concrete_io_dowork;
    IO_SETOPTION concrete_io_setoption;
} IO_INTERFACE_DESCRIPTION;
```
 **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_002: [** The tlsio_openssl_compact shall report the open operation status using the IO_OPEN_RESULT enumerator defined in the `xio.h`:
```c
typedef enum IO_OPEN_RESULT_TAG
{
    IO_OPEN_OK,
    IO_OPEN_ERROR,
    IO_OPEN_CANCELLED
} IO_OPEN_RESULT;
```
 **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_003: [** The tlsio_openssl_compact shall report the send operation status using the IO_SEND_RESULT enumerator defined in the `xio.h`:
```c
typedef enum IO_SEND_RESULT_TAG
{
    IO_SEND_OK,
    IO_SEND_ERROR,
    IO_SEND_CANCELLED
} IO_SEND_RESULT;
```
 **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_004: [** The tlsio_openssl_compact shall call the callbacks functions defined in the `xio.h`:
```c
typedef void(*ON_BYTES_RECEIVED)(void* context, const unsigned char* buffer, size_t size);
typedef void(*ON_SEND_COMPLETE)(void* context, IO_SEND_RESULT send_result);
typedef void(*ON_IO_OPEN_COMPLETE)(void* context, IO_OPEN_RESULT open_result);
typedef void(*ON_IO_CLOSE_COMPLETE)(void* context);
typedef void(*ON_IO_ERROR)(void* context);
```
 **]**

 **SRS_TLSIO_OPENSSL_COMPACT_30_005: [** The tlsio_openssl_compact shall receive the connection information using the TLSIO_CONFIG structure defined in `tlsio.h`:
 ```c
 typedef struct TLSIO_CONFIG_TAG
 {
 	const char* hostname;
 	int port;
 } TLSIO_CONFIG;
 ```
  **]**

  **SRS_TLSIO_OPENSSL_COMPACT_30_006: [** The tlsio_openssl_compact shall observe this internally defined timeout for its opening and sending processes. This value is considered an emergency limit rather than a useful tuning parameter, so it is not adjustable via the more expensive get / set options system:
  ```c
#ifndef TLSIO_OPERATION_TIMEOUT_SECONDS
#define TLSIO_OPERATION_TIMEOUT_SECONDS 40
#endif // !TLSIO_OPERATION_TIMEOUT_SECONDS
  ```
   **]**

## API Calls

###   tlsio_openssl_compact_get_interface_description
```c
const IO_INTERFACE_DESCRIPTION* tlsio_openssl_compact_get_interface_description(void);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_008: [** The tlsio_get_interface_description shall return the VTable IO_INTERFACE_DESCRIPTION. **]**


###   tlsio_openssl_compact_create
Implementation of `IO_CREATE concrete_io_create`
```c
CONCRETE_IO_HANDLE tlsio_openssl_compact_create(void* io_create_parameters);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_010: [** The `tlsio_openssl_compact_create` shall allocate and initialize all necessary resources and return an instance of the `tlsio_openssl_compact`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_011: [** If any resource allocation fails, `tlsio_openssl_compact_create` shall return NULL. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_012: [** The `tlsio_openssl_compact_create` shall receive the connection configuration as a `TLSIO_CONFIG*` in `io_create_parameters`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_013: [** If the `io_create_parameters` value is NULL, `tlsio_openssl_compact_create` shall log an error and return NULL. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_014: [** If the `hostname` member of `io_create_parameters` value is NULL, `tlsio_openssl_compact_create` shall log an error and return NULL. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_015: [** If the `port` member of `io_create_parameters` value is less than 0 or greater than 0xffff, `tlsio_openssl_compact_create` shall log an error and return NULL. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_016: [** `tlsio_openssl_compact_create` shall make a copy of the `hostname` member of `io_create_parameters` to allow deletion of `hostname` immediately after the call. **]**


###   tlsio_openssl_compact_destroy
Implementation of `IO_DESTROY concrete_io_destroy`
```c
void tlsio_openssl_compact_destroy(CONCRETE_IO_HANDLE tlsio_handle);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_020: [** If `tlsio_handle` is NULL, `tlsio_openssl_compact_destroy` shall do nothing. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_021: [** The `tlsio_openssl_compact_destroy` shall release all allocated resources and then release `tlsio_handle`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_022: [** If `tlsio_openssl_compact_close` has not been called before `concrete_io_destroy`, `concrete_io_destroy` shall additionally log an error. **]**


###   tlsio_openssl_compact_open
Implementation of `IO_OPEN concrete_io_open`
```c
int tlsio_openssl_compact_open(
    CONCRETE_IO_HANDLE tlsio_handle,
    ON_IO_OPEN_COMPLETE on_io_open_complete,
    void* on_io_open_complete_context,
    ON_BYTES_RECEIVED on_bytes_received,
    void* on_bytes_received_context,
    ON_IO_ERROR on_io_error,
    void* on_io_error_context);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_030: [** If the `tlsio_handle` parameter is NULL, `tlsio_openssl_compact_open` shall log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_031: [** If the on_io_open_complete parameter is NULL, `tlsio_openssl_compact_open` shall log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_032: [** If the on_bytes_received parameter is NULL, `tlsio_openssl_compact_open` shall log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_033: [** If the on_io_error parameter is NULL, `tlsio_openssl_compact_open` shall log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_034: [** The `tlsio_openssl_compact_open` shall store the provided `on_bytes_received`,  `on_bytes_received_context`, `on_io_error`, `on_io_error_context`, `on_io_open_complete`,  and `on_io_open_complete_context` parameters for later use as specified and tested per other line entries in this document. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_035: [** The `tlsio_openssl_compact_open` shall begin the process of opening the ssl connection with the host provided in the `tlsio_openssl_compact_create` call. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_036: [** If `tlsio_openssl_compact_open` successfully begins opening the OpenSSL connection, it shall return 0. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_037: [** If `tlsio_openssl_compact_open` has already been called, it shall log an error, and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_038: [** If the `tlsio_openssl_compact_open` returns _FAILURE_  it shall call `on_io_open_complete` with the provided `on_io_open_complete_context` and IO_OPEN_ERROR. **]**


###   tlsio_openssl_compact_close
Implementation of `IO_CLOSE concrete_io_close`
```c
int tlsio_openssl_compact_close(CONCRETE_IO_HANDLE tlsio_handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_050: [** If the `tlsio_handle` parameter is NULL, `tlsio_openssl_compact_close` shall log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_051: [** The `tlsio_openssl_compact_close` shall forcibly close any existing ssl connection. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_052: [** The `tlsio_openssl_compact_close` return value shall be 0 except as noted in the next requirement. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_053: [** If `tlsio_openssl_compact_open` has not been called previously then `tlsio_openssl_compact_close` shall log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_054: [** If `tlsio_openssl_compact_open` has been called  but the process of opening has not been completed, then the `on_io_open_complete` callback shall be made with `IO_OPEN_CANCELLED`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_056: [** If `tlsio_openssl_compact_close` is called while there are unsent messages in the queue, the `tlsio_openssl_compact_close` shall call each message's  `on_send_complete`, passing its associated `callback_context` and `IO_SEND_CANCELLED`. **]**


###   tlsio_openssl_compact_send
Implementation of `IO_SEND concrete_io_send`
```c
int tlsio_openssl_compact_send(CONCRETE_IO_HANDLE tlsio_handle, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_060: [** If the `tlsio_handle` parameter is NULL, `tlsio_openssl_compact_send` shall log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_061: [** If the `buffer` is NULL, `tlsio_openssl_compact_send` shall log the error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_062: [** If the `on_send_complete` is NULL, `tlsio_openssl_compact_send` shall log the error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_063: [** The `tlsio_openssl_compact_send` shall enqueue for transmission the `on_send_complete`, the `callback_context`, the `size`, and the contents of `buffer`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_064: [** If the supplied message cannot be enqueued for transmission, `tlsio_openssl_compact_send` shall call the `on_send_complete` with IO_SEND_ERROR, and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_065: [** If `tlsio_openssl_compact_open` has not been called or the opening process has not been completed, `tlsio_openssl_compact_send` shall log an error and return _FAILURE_. **]**

###   tlsio_openssl_compact_dowork
Implementation of `IO_DOWORK concrete_io_dowork`
```c
void tlsio_openssl_compact_dowork(CONCRETE_IO_HANDLE tlsio_handle);
```
The `tlsio_openssl_compact_dowork` call executes async jobs for the tlsio. This includes connection completion, sending to the SSL connection, and checking the SSL connection for available bytes to read.

The underlying OpenSSL `SSL_read` call does not return errors if the connection has been lost, so the tlsio will not know if the connection has been lost until it attempts a write operation.

**SRS_TLSIO_OPENSSL_COMPACT_30_070: [** If the `tlsio_handle` parameter is NULL, `tlsio_openssl_compact_dowork` shall do nothing except log an error. **]**

#### Post-error behavior

These post-error behavior specifications take precedence over all other behavior specifications.

**SRS_TLSIO_OPENSSL_COMPACT_30_071: [** If the `tlsio_openssl_compact` adapter has returned a failure from `tlsio_openssl_compact_open` or if `on_io_open_complete` has been called with `IO_OPEN_ERROR` or `IO_OPEN_CANCELLED`, then `tlsio_openssl_compact_dowork` shall do nothing. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_072: [** If `tlsio_openssl_compact_dowork` ever calls `on_io_error`, then subsequent calls to `tlsio_openssl_compact_dowork` shall do nothing. **]**

#### Behavior selection

**SRS_TLSIO_OPENSSL_COMPACT_30_075: [** If `tlsio_openssl_compact_dowork` is called before `tlsio_openssl_compact_open`, `tlsio_openssl_compact_dowork` shall do nothing. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_076: [** If `tlsio_openssl_compact_dowork` is called after `tlsio_openssl_compact_close`, `tlsio_openssl_compact_dowork` shall do nothing. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_077: [** If  `tlsio_openssl_compact_dowork` called after `tlsio_openssl_compact_open` but before the connection process completes, `tlsio_openssl_compact_dowork` shall perform the [Connection completion behaviors](#connection-completion-behaviors) but not the [Data transmission behaviors](#data-transmission-behaviors) or the [Data reception behaviors](#data-reception-behaviors). **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_078: [** If `tlsio_openssl_compact_dowork` is called after the connection process completes successfully, `tlsio_openssl_compact_dowork` shall perform the [Data transmission behaviors](#data-transmission-behaviors) and then the [Data reception behaviors](#data-reception-behaviors) but not the [Connection completion behaviors](#connection-completion-behaviors). **]**

#### Connection completion behaviors

Connection completion may require multiple calls to `tlsio_openssl_compact_dowork`. The number of calls required is not specified.

**SRS_TLSIO_OPENSSL_COMPACT_30_080: [** The `tlsio_openssl_compact_dowork` shall establish an OpenSSL connection using the `hostName` and `port` provided during `tlsio_openssl_compact_open`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_081: [** If the connection process takes longer than the internally defined `TLSIO_OPERATION_TIMEOUT_SECONDS` it shall log an error and call `on_io_open_complete` with the `on_io_open_complete_context` parameter provided in `tlsio_openssl_compact_open` and `IO_OPEN_ERROR`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_082: [** If the connection process fails for any reason, `tlsio_openssl_compact_dowork` shall log an error and call `on_io_open_complete` with the `on_io_open_complete_context` parameter provided in `tlsio_openssl_compact_open` and `IO_OPEN_ERROR`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_083: [** If `tlsio_openssl_compact_dowork` successfully opens the ssl connection it shall call `on_io_open_complete` with the `on_io_open_complete_context` parameter provided in `tlsio_openssl_compact_open` and `IO_OPEN_OK`. **]**

#### Data transmission behaviors

**SRS_TLSIO_OPENSSL_COMPACT_30_090: [** If an enqueued message size is 0, the `tlsio_openssl_compact_dowork` shall just call the `on_send_complete` with `IO_SEND_OK`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_091: [** If `tlsio_openssl_compact_dowork` is able to send all the bytes in an enqueued message, it shall call the messages's `on_send_complete` along with its associated `callback_context` and `IO_SEND_OK`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_092: [** If the send process for any given message takes longer than the internally defined `TLSIO_OPERATION_TIMEOUT_SECONDS` it call the message's `on_send_complete` along with its associated `callback_context` and `IO_SEND_ERROR`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_093: [** If the OpenSSL send was not able to send an entire enqueued message at once, subsequent calls to `tlsio_openssl_compact_dowork` shall repeat OpenSSL send for the remaining bytes. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_094: [** The `tlsio_openssl_compact_dowork` shall supply the provided `callback_context` when it calls `on_send_complete`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_095: [** If the send process fails before sending all of the bytes in an enqueued message, the `tlsio_openssl_compact_dowork` shall call the message's `on_send_complete` along with its associated `callback_context` and `IO_SEND_ERROR`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_096: [** If there are no enqueued messages available, `tlsio_openssl_compact_dowork` shall do nothing. **]**

#### Data reception behaviors

The underlying API for OpenSSL is not capable of detecting useful errors when attempting to receive, so it will never call the `on_io_error` callback supplied during `tlsio_openssl_compact_open`.

**SRS_TLSIO_OPENSSL_COMPACT_30_100: [** If the OpenSSL client is able to provide received data, the `tlsio_openssl_compact_dowork` shall read this data and call  `on_bytes_received` with the pointer to the buffer containing the data and the number of bytes received. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_101: [** When `tlsio_openssl_compact_dowork` calls `on_bytes_received`, it shall pass the `on_bytes_received_context` handle as a parameter. **]**


###   tlsio_openssl_compact_setoption
Implementation of `IO_SETOPTION concrete_io_setoption`
```c
int tlsio_openssl_compact_setoption(CONCRETE_IO_HANDLE tlsio_handle, const char* optionName, const void* value);
```
**SRS_TLSIO_OPENSSL_COMPACT_30_120: [** If the `tlsio_handle` parameter is NULL, `tlsio_openssl_compact_setoption` shall do nothing except log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_121: [** If the `optionName` parameter is NULL, `tlsio_openssl_compact_setoption` shall do nothing except log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_122: [** If the `value` parameter is NULL, `tlsio_openssl_compact_setoption` shall do nothing except log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_123 [** The `tlsio_openssl_compact_setoption` shall do nothing and return 0. **]**


###   tlsio_openssl_compact_retrieveoptions
Implementation of `IO_RETRIEVEOPTIONS concrete_io_retrieveoptions`
```c
OPTIONHANDLER_HANDLE tlsio_openssl_compact_retrieveoptions(CONCRETE_IO_HANDLE tlsio_handle);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_160: [** If the `tlsio_handle` parameter is NULL, `tlsio_openssl_compact_retrieveoptions` shall do nothing except log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_161: [** The `tlsio_openssl_compact_retrieveoptions` shall do nothing and return NULL. **]**
