tlsio_openssl_compact
=============

## Overview

tlsio_openssl_compact implements a tlsio adapter for compact OpenSSL platforms.  


## References

[TLS Protocol (RFC2246)](https://www.ietf.org/rfc/rfc2246.txt)

[TLS Protocol (generic information)](https://en.wikipedia.org/wiki/Transport_Layer_Security)


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


## Callbacks

**SRS_TLSIO_OPENSSL_COMPACT_30_006: [** The tlsio_openssl_compact shall return the status of all async operations using the callbacks. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_007: [** If the callback function is set as NULL. The tlsio_openssl_compact shall not call anything. **]**

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

**SRS_TLSIO_OPENSSL_COMPACT_30_009: [** The `tlsio_openssl_compact_create` shall allocate, initialize, and return an instance of the tlsio for compact OpenSSL. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_010: [** If the allocation fails, `tlsio_openssl_compact_create` shall return NULL. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_011: [** The `tlsio_openssl_compact_create` shall initialize all internal callback pointers as NULL. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_012: [** The `tlsio_openssl_compact_create` shall receive the connection configuration (TLSIO_CONFIG). **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_013: [** If the `io_create_parameters` value is NULL, `tlsio_openssl_compact_create` shall log an error and return NULL. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_014: [** The `tlsio_openssl_compact_create` shall convert the provided `hostName` to an IPv4 address. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_015: [** If the IP for the `hostName` cannot be found, `tlsio_openssl_compact_create` shall return NULL. **]**


###   tlsio_openssl_compact_destroy
Implementation of `IO_DESTROY concrete_io_destroy`
```c
void tlsio_openssl_compact_destroy(CONCRETE_IO_HANDLE tlsio_handle);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_016: [** If `tlsio_handle` is NULL, `tlsio_openssl_compact_destroy` shall do nothing. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_017: [** The `tlsio_openssl_compact_destroy` shall release tlsio_handle and all its associated resources. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_018: [** If `tlsio_openssl_compact_close` has not been called immediately prior to `tlsio_openssl_compact_destroy`, the method shall release  tlsio_handle and all its associated resources and log an error. **]**


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

**SRS_TLSIO_OPENSSL_COMPACT_30_019: [** If the `tlsio_handle` parameter is NULL, `tlsio_openssl_compact_open` shall do nothing except log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_020: [** If the on_bytes_received parameter is NULL, `tlsio_openssl_compact_open` shall log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_021: [** The `tlsio_openssl_compact_open` shall open the ssl connection with the host provided in the `tlsio_openssl_compact_create`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_022: [** The `tlsio_openssl_compact_open` shall store the provided `on_bytes_received` callback function address. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_023: [** The `tlsio_openssl_compact_open` shall store the provided `on_bytes_received_context` handle. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_024: [** The `tlsio_openssl_compact_open` shall store the provided `on_io_error` callback function address. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_025: [** The `tlsio_openssl_compact_open` shall store the provided `on_io_error_context` handle. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_026: [** If `tlsio_openssl_compact_open` successfully opens the ssl connection, it shall return 0. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_027: [** If `tlsio_openssl_compact_open` successfully opens the ssl connection and `on_io_open_complete` is non-NULL it shall call `on_io_open_complete` with IO_OPEN_OK. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_028: [** If `tlsio_openssl_compact_open` calls `on_io_open_complete`, it shall always pass the provided `on_io_open_complete_context` parameter. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_029: [** If either `tlsio_openssl_compact_create` or `tlsio_openssl_compact_close` have not been called immediately prior to `tlsio_openssl_compact_open`, then `tlsio_openssl_compact_open` shall return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_030: [** If `tlsio_openssl_compact_open` fails to open the ssl connection, it shall return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_031: [** If the `tlsio_openssl_compact_open` fails to open the tls connection, and the `on_io_open_complete` callback was provided, it shall call  `on_io_open_complete` with IO_OPEN_ERROR. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_032: [** If the `tlsio_openssl_compact_open` fails to open the tls connection, and the `on_io_error` callback was provided, it shall call  `on_io_error` and pass in the provided `on_io_error_context`. **]**


###   tlsio_openssl_compact_close
Implementation of `IO_CLOSE concrete_io_close`
```c
int tlsio_openssl_compact_close(CONCRETE_IO_HANDLE tlsio_handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_033: [** If the `tlsio_handle` parameter is NULL, `tlsio_openssl_compact_close` shall do nothing except log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_034: [** The `tlsio_openssl_compact_close` shall always forcibly close any existing ssl connection. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_035: [** The `tlsio_openssl_compact_close` return value shall be 0 except as noted in the next requirement. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_036: [** If either `tlsio_openssl_compact_close` or `tlsio_openssl_compact_create` was called immediately prior to `tlsio_openssl_compact_close`, then `tlsio_openssl_compact_close` shall log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_037: [** If `on_io_close_complete` is provided, `tlsio_openssl_compact_close` shall call `on_io_close_complete`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_038: [** If on_io_close_complete is provided,  `tlsio_openssl_compact_close` shall pass the `callback_context` handle into the `on_io_close_complete` call. **]**


###   tlsio_openssl_compact_send
Implementation of `IO_SEND concrete_io_send`
```c
int tlsio_openssl_compact_send(CONCRETE_IO_HANDLE tlsio_handle, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_039: [** If the `tlsio_handle` parameter is NULL, `tlsio_openssl_compact_send` shall do nothing except log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_040: [** The `tlsio_openssl_compact_send` shall send the first `size` bytes in `buffer` to the ssl connection. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_041: [** The `tlsio_openssl_compact_send` shall call the provided `on_send_complete` callback function. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_042: [** The `tlsio_openssl_compact_send` shall supply the provided `callback_context` when it calls `on_send_complete`. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_043: [** if the ssl was not able to send all data in the buffer, the `tlsio_openssl_compact_send` shall call the ssl again to send the remaining bytes. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_044: [** if the ssl fails before sending all of the bytes in the buffer, the `tlsio_openssl_compact_send` shall call the `on_send_complete` with IO_SEND_ERROR, and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_045: [** if the ssl was able to send all the bytes in the buffer, the `tlsio_openssl_compact_send` shall call the `on_send_complete` with IO_SEND_OK, and return 0 **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_046: [** If the buffer is NULL, the `tlsio_openssl_compact_send` shall do nothing except log the error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_047: [** If the size is 0, the `tlsio_openssl_compact_send` shall do nothing and return 0. **]**


###   tlsio_openssl_compact_dowork
Implementation of `IO_DOWORK concrete_io_dowork`
```c
void tlsio_openssl_compact_dowork(CONCRETE_IO_HANDLE tlsio_handle);
```
The `tlsio_openssl_compact_dowork` call executes async jobs for the tlsio. For this implementation, the only async work required is to check the SSL connection for available bytes to read.

The underlying OpenSSL `SSL_read` call does not return errors if the connection has been lost, so the tlsio will not know if the connection has been lost until it attempts a write operation.

**SRS_TLSIO_OPENSSL_COMPACT_30_048: [** If the `tlsio_handle` parameter is NULL, `tlsio_openssl_compact_dowork` shall do nothing except log an error and return _FAILURE_. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_049: [** If the ssl client is able to provide received data, the `tlsio_openssl_compact_dowork` shall read this data and call  `on_bytes_received` with the pointer to the buffer containing the data and the number of bytes received. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_050: [** When `tlsio_openssl_compact_dowork` calls `on_bytes_received`, it shall pass the `on_bytes_received_context` handle as a parameter. **]**

**SRS_TLSIO_OPENSSL_COMPACT_30_051: [** The `tlsio_openssl_compact_dowork` shall use a stack-based buffer to store the data received from the ssl client. **]**


###   tlsio_openssl_compact_setoption
Implementation of `IO_SETOPTION concrete_io_setoption`
```c
int tlsio_openssl_compact_setoption(CONCRETE_IO_HANDLE tlsio_handle, const char* optionName, const void* value);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_052 [** The `tlsio_openssl_compact_setoption` shall do nothing and return 0. **]**


###   tlsio_openssl_compact_retrieveoptions
Implementation of `IO_RETRIEVEOPTIONS concrete_io_retrieveoptions`
```c
OPTIONHANDLER_HANDLE tlsio_openssl_compact_retrieveoptions(CONCRETE_IO_HANDLE tlsio_handle);
```

**SRS_TLSIO_OPENSSL_COMPACT_30_053: [** The `tlsio_openssl_compact_retrieveoptions` shall do nothing and return NULL. **]**
