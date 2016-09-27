tlsio_arduino
=============

## Overview

tlsio_arduino implements a tls adapter for the Arduino platform.  


## References

[TLS Protocol (RFC2246)](https://www.ietf.org/rfc/rfc2246.txt)

[TLS Protocol (generic information)](https://en.wikipedia.org/wiki/Transport_Layer_Security)

[tlsio_arduino.pptx]

## Exposed API

**SRS_TLSIO_ARDUINO_21_001: [** The tlsio_arduino shall implement and export all the Concrete functions in the VTable IO_INTERFACE_DESCRIPTION defined in the `xio.h`. 
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

**SRS_TLSIO_ARDUINO_21_002: [** The tlsio_arduino shall report the open operation status using the IO_OPEN_RESULT enumerator defined in the `xio.h`:
```c
typedef enum IO_OPEN_RESULT_TAG
{
    IO_OPEN_OK,
    IO_OPEN_ERROR,
    IO_OPEN_CANCELLED
} IO_OPEN_RESULT;
```
**]**

**SRS_TLSIO_ARDUINO_21_003: [** The tlsio_arduino shall report the send operation status using the IO_SEND_RESULT enumerator defined in the `xio.h`:
```c
typedef enum IO_SEND_RESULT_TAG
{
    IO_SEND_OK,
    IO_SEND_ERROR,
    IO_SEND_CANCELLED
} IO_SEND_RESULT;
```
**]**

**SRS_TLSIO_ARDUINO_21_004: [** The tlsio_arduino shall call the callbacks functions defined in the `xio.h`:
```c
typedef void(*ON_BYTES_RECEIVED)(void* context, const unsigned char* buffer, size_t size);
typedef void(*ON_SEND_COMPLETE)(void* context, IO_SEND_RESULT send_result);
typedef void(*ON_IO_OPEN_COMPLETE)(void* context, IO_OPEN_RESULT open_result);
typedef void(*ON_IO_CLOSE_COMPLETE)(void* context);
typedef void(*ON_IO_ERROR)(void* context);
```
**]**

**SRS_TLSIO_ARDUINO_21_005: [** The tlsio_arduino shall received the connection information using the TLSIO_CONFIG structure defined in `tlsio.h`:
```c
typedef struct TLSIO_CONFIG_TAG
{
	const char* hostname;
	int port;
} TLSIO_CONFIG;
```
**]**


## Callbacks

**SRS_TLSIO_ARDUINO_21_006: [** The tlsio_arduino shall return the status of all async operations using the callbacks. **]**

**SRS_TLSIO_ARDUINO_21_007: [** If the callback function is set as NULL. The tlsio_arduino shall not call anything. **]**


###  tlsio_arduino_get_interface_description
```c
const IO_INTERFACE_DESCRIPTION* tlsio_arduino_get_interface_description(void);
```

**SRS_TLSIO_ARDUINO_21_008: [** The tlsio_arduino_get_interface_description shall return the VTable IO_INTERFACE_DESCRIPTION. **]**


###  tlsio_arduino_create
Implementation of `IO_CREATE concrete_io_create`
```c
CONCRETE_IO_HANDLE tlsio_arduino_create(void* io_create_parameters);
```

**SRS_TLSIO_ARDUINO_21_009: [** The tlsio_arduino_create shall create a new instance of the tlsio for Arduino. **]**

**SRS_TLSIO_ARDUINO_21_010: [** The tlsio_arduino_create shall return a non-NULL handle on success. **]**

**SRS_TLSIO_ARDUINO_21_011: [** The tlsio_arduino_create shall allocate memory to control the tlsio instance. **]**

**SRS_TLSIO_ARDUINO_21_012: [** If there is not enough memory to control the tlsio, the tlsio_arduino_create shall return NULL as the handle. **]**

**SRS_TLSIO_ARDUINO_21_015: [** The tlsio_arduino_create shall set 10 seconds for the sslClient timeout. **]**

**SRS_TLSIO_ARDUINO_21_016: [** The tlsio_arduino_create shall initialize all callback pointers as NULL. **]**

**SRS_TLSIO_ARDUINO_21_017: [** The tlsio_arduino_create shall receive the connection configuration (TLSIO_CONFIG). **]**

**SRS_TLSIO_ARDUINO_21_018: [** The tlsio_arduino_create shall convert the provide hostName to an IP address. **]**

**SRS_TLSIO_ARDUINO_21_019: [** If the WiFi cannot find the IP for the hostName, the tlsio_arduino_create shall destroy the sslClient and tlsio instances and return NULL as the handle. **]**

**SRS_TLSIO_ARDUINO_21_020: [** If tlsio_arduino_create get success to create the tlsio instance, it shall set the tlsio state as TLSIO_ARDUINO_STATE_CLOSED. **]**


###  tlsio_arduino_destroy
Implementation of `IO_DESTROY concrete_io_destroy`
```c
void tlsio_arduino_destroy(CONCRETE_IO_HANDLE tlsio_handle);
```

**SRS_TLSIO_ARDUINO_21_021: [** The tlsio_arduino_destroy shall destroy a created instance of the tlsio for Arduino identified by the CONCRETE_IO_HANDLE. **]**

**SRS_TLSIO_ARDUINO_21_022: [** The tlsio_arduino_destroy shall free the memory allocated for tlsio_instance. **]**

**SRS_TLSIO_ARDUINO_21_024: [** If the tlsio_handle is NULL, the tlsio_arduino_destroy shall not do anything. **]**

**SRS_TLSIO_ARDUINO_21_025: [** If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_OPEN, or TLSIO_ARDUINO_STATE_CLOSING, the tlsio_arduino_destroy shall close destroy the tlsio, but log an error. **]**


###  tlsio_arduino_open
Implementation of `IO_OPEN concrete_io_open`
```c
int tlsio_arduino_open(
    CONCRETE_IO_HANDLE tlsio_handle, 
    ON_IO_OPEN_COMPLETE on_io_open_complete, 
    void* on_io_open_complete_context, 
    ON_BYTES_RECEIVED on_bytes_received, 
    void* on_bytes_received_context, 
    ON_IO_ERROR on_io_error, 
    void* on_io_error_context);
```

**SRS_TLSIO_ARDUINO_21_026: [** The tlsio_arduino_open shall star the process to open the ssl connection with the host provided in the tlsio_arduino_create. **]**

**SRS_TLSIO_ARDUINO_21_027: [** The tlsio_arduino_open shall set the tlsio to try to open the connection for 10 times before assuming that connection failed. **]**

**SRS_TLSIO_ARDUINO_21_028: [** The tlsio_arduino_open shall store the provided on_io_open_complete callback function address. **]**

**SRS_TLSIO_ARDUINO_21_029: [** The tlsio_arduino_open shall store the provided on_io_open_complete_context handle. **]**

**SRS_TLSIO_ARDUINO_21_030: [** The tlsio_arduino_open shall store the provided on_bytes_received callback function address. **]**

**SRS_TLSIO_ARDUINO_21_031: [** The tlsio_arduino_open shall store the provided on_bytes_received_context handle. **]**

**SRS_TLSIO_ARDUINO_21_032: [** The tlsio_arduino_open shall store the provided on_io_error callback function address. **]**

**SRS_TLSIO_ARDUINO_21_033: [** The tlsio_arduino_open shall store the provided on_io_error_context handle. **]**

**SRS_TLSIO_ARDUINO_21_034: [** If tlsio_arduino_open get success to start the process to open the ssl connection, it shall set the tlsio state as TLSIO_ARDUINO_STATE_OPENING, and return 0. **]**

**SRS_TLSIO_ARDUINO_21_035: [** If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_OPEN, or TLSIO_ARDUINO_STATE_CLOSING, the tlsio_arduino_open shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, and return _LINE_. **]**

**SRS_TLSIO_ARDUINO_21_036: [** If the tlsio_handle is NULL, the tlsio_arduino_open shall not do anything, and return _LINE_. **]**

**SRS_TLSIO_ARDUINO_21_037: [** If the ssl client is connected, the tlsio_arduino_open shall change the state to TLSIO_ARDUINO_STATE_ERROR, log the error, and return _LINE_. **]**

**SRS_TLSIO_ARDUINO_21_038: [** If tlsio_arduino_open failed to start the process to open the ssl connection, it shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, and return _LINE_. **]**

**SRS_TLSIO_ARDUINO_21_039: [** If the tlsio_arduino_open failed to open the tls connection, and the on_io_open_complete callback was provided, it shall call the on_io_open_complete with IO_OPEN_ERROR. **]**

**SRS_TLSIO_ARDUINO_21_040: [** If the tlsio_arduino_open failed to open the tls connection, and the on_io_error callback was provided, it shall call the on_io_error. **]**

**SRS_TLSIO_ARDUINO_21_041: [** If the tlsio_arduino_open get success opening the tls connection, it shall call the tlsio_arduino_dowork. **]**

**SRS_TLSIO_ARDUINO_21_042: [** If the tlsio_arduino_open retry to open more than 10 times without success, it shall call the on_io_open_complete with IO_OPEN_CANCELED. **]**


###  tlsio_arduino_close
Implementation of `IO_CLOSE concrete_io_close`
```c
int tlsio_arduino_close(CONCRETE_IO_HANDLE tlsio_handle, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context);
```

**SRS_TLSIO_ARDUINO_21_043: [** The tlsio_arduino_close shall start the process to close the ssl connection. **]**

**SRS_TLSIO_ARDUINO_21_044: [** The tlsio_arduino_close shall set the tlsio to try to close the connection for 10 times before assuming that close connection failed. **]**

**SRS_TLSIO_ARDUINO_21_045: [** The tlsio_arduino_close shall store the provided on_io_close_complete callback function address. **]**

**SRS_TLSIO_ARDUINO_21_046: [** The tlsio_arduino_close shall store the provided on_io_close_complete_context handle. **]**

**SRS_TLSIO_ARDUINO_21_047: [** If tlsio_arduino_close get success to start the process to close the ssl connection, it shall set the tlsio state as TLSIO_ARDUINO_STATE_CLOSING, and return 0. **]**

**SRS_TLSIO_ARDUINO_21_048: [** If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_close shall set the tlsio state as TLSIO_ARDUINO_STATE_ERROR, call the on_io_error, and return _LINE_. **]**

**SRS_TLSIO_ARDUINO_21_049: [** If the tlsio_handle is NULL, the tlsio_arduino_close shall not do anything, and return _LINE_. **]**

**SRS_TLSIO_ARDUINO_21_050: [** If the tlsio_arduino_close get success closing the tls connection, it shall call the tlsio_arduino_dowork. **]**

**SRS_TLSIO_ARDUINO_21_051: [** If the tlsio_arduino_close retry to close more than 10 times without success, it shall call the on_io_error. **]**


###  tlsio_arduino_send
Implementation of `IO_SEND concrete_io_send`
```c
int tlsio_arduino_send(CONCRETE_IO_HANDLE tlsio_handle, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
```

**SRS_TLSIO_ARDUINO_21_052: [** The tlsio_arduino_send shall send all bytes in a buffer to the ssl connectio. **]**

**SRS_TLSIO_ARDUINO_21_053: [** The tlsio_arduino_send shall use the provided on_io_send_complete callback function address. **]**

**SRS_TLSIO_ARDUINO_21_054: [** The tlsio_arduino_send shall use the provided on_io_send_complete_context handle. **]**

**SRS_TLSIO_ARDUINO_21_055: [** if the ssl was not able to send all data in the buffer, the tlsio_arduino_send shall call the ssl again to send the remaining bytes. **]**

**SRS_TLSIO_ARDUINO_21_056: [** if the ssl was not able to send any byte in the buffer, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. **]**

**SRS_TLSIO_ARDUINO_21_057: [** if the ssl finish to send all bytes in the buffer, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_OK, and return 0 **]**

**SRS_TLSIO_ARDUINO_21_058: [** If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, TLSIO_ARDUINO_STATE_OPENING, TLSIO_ARDUINO_STATE_CLOSING, or TLSIO_ARDUINO_STATE_CLOSED, the tlsio_arduino_send shall call the on_send_complete with IO_SEND_ERROR, and return _LINE_. **]**

**SRS_TLSIO_ARDUINO_21_059: [** If the tlsio_handle is NULL, the tlsio_arduino_send shall not do anything, and return _LINE_. **]**

**SRS_TLSIO_ARDUINO_21_060: [** If the buffer is NULL, the tlsio_arduino_send shall not do anything, and return _LINE_. **]**

**SRS_TLSIO_ARDUINO_21_061: [** If the size is 0, the tlsio_arduino_send shall not do anything, and return _LINE_. **]**


###  tlsio_arduino_dowork
Implementation of `IO_DOWORK concrete_io_dowork`
```c
void tlsio_arduino_dowork(CONCRETE_IO_HANDLE tlsio_handle)
```

**SRS_TLSIO_ARDUINO_21_062: [** The tlsio_arduino_dowork shall execute the async jobs for the tlsio. **]**

**SRS_TLSIO_ARDUINO_21_063: [** If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, and ssl client is connected, the tlsio_arduino_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_OPEN, and call the on_io_open_complete with IO_OPEN_OK. **]**

**SRS_TLSIO_ARDUINO_21_064: [** If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, and ssl client is not connected, the tlsio_arduino_dowork shall decrement the counter of trys for opening. **]**

**SRS_TLSIO_ARDUINO_21_065: [** If the tlsio state is TLSIO_ARDUINO_STATE_OPENING, ssl client is not connected, and the counter to try becomes 0, the tlsio_arduino_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_ERROR, call on_io_open_complete with IO_OPEN_CANCELLED, call on_io_error. **]**

**SRS_TLSIO_ARDUINO_21_066: [** If the tlsio state is TLSIO_ARDUINO_STATE_CLOSING, and ssl client is not connected, the tlsio_arduino_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_CLOSE, and call the on_io_close_complete. **]**

**SRS_TLSIO_ARDUINO_21_067: [** If the tlsio state is TLSIO_ARDUINO_STATE_CLOSING, and ssl client is connected, the tlsio_arduino_dowork shall decrement the counter of trys for closing. **]**

**SRS_TLSIO_ARDUINO_21_068: [** If the tlsio state is TLSIO_ARDUINO_STATE_CLOSING, ssl client is connected, and the counter to try becomes 0, the tlsio_arduino_dowork shall change the tlsio state to TLSIO_ARDUINO_STATE_ERROR, call on_io_error. **]**

**SRS_TLSIO_ARDUINO_21_069: [** If the tlsio state is TLSIO_ARDUINO_STATE_OPEN, the tlsio_arduino_dowork shall read data from the ssl client. **]**

**SRS_TLSIO_ARDUINO_21_070: [** If the tlsio state is TLSIO_ARDUINO_STATE_OPEN, and there are received data in the ssl client, the tlsio_arduino_dowork shall read this data and call the on_bytes_received with the pointer to the buffer with the data. **]**

**SRS_TLSIO_ARDUINO_21_071: [** If the tlsio state is TLSIO_ARDUINO_STATE_OPEN, and ssl client is not connected, the tlsio_arduino_dowork shall change the state to TLSIO_ARDUINO_STATE_ERROR, call on_io_error. **]**

**SRS_TLSIO_ARDUINO_21_072: [** If the tlsio state is TLSIO_ARDUINO_STATE_CLOSED, and ssl client is connected, the tlsio_arduino_dowork shall change the state to TLSIO_ARDUINO_STATE_ERROR, call on_io_error. **]**

**SRS_TLSIO_ARDUINO_21_073: [** If the tlsio state is TLSIO_ARDUINO_STATE_ERROR, the tlsio_arduino_dowork shall not do anything. **]**

**SRS_TLSIO_ARDUINO_21_074: [** If the tlsio_handle is NULL, the tlsio_arduino_dowork shall not do anything. **]**

**SRS_TLSIO_ARDUINO_21_075: [** The tlsio_arduino_dowork shall create a buffer to store the data received from the ssl client. **]**

**SRS_TLSIO_ARDUINO_21_076: [** The tlsio_arduino_dowork shall delete the buffer to store the data received from the ssl client. **]**


###  tlsio_arduino_setoption
Implementation of `IO_SETOPTION concrete_io_setoption`
```c
int tlsio_arduino_setoption(CONCRETE_IO_HANDLE tlsio_handle, const char* optionName, const void* value)
```

**SRS_TLSIO_ARDUINO_21_077: [** The tlsio_arduino_setoption shall not do anything, and return 0. **]**


###  tlsio_arduino_retrieveoptions
Implementation of `IO_RETRIEVEOPTIONS concrete_io_retrieveoptions`
```c
OPTIONHANDLER_HANDLE tlsio_arduino_retrieveoptions(CONCRETE_IO_HANDLE tlsio_handle)
```

**SRS_TLSIO_ARDUINO_21_078: [** The tlsio_arduino_retrieveoptions shall not do anything, and return NULL. **]**


