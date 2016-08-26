platform_arduino
=============

## Overview

platform_arduino implements a standart way for SDK to access dedicated Arduino interfaces.  

## References


###  Standard

**SRS_PLATFORM_ARDUINO_21_001: [** The platform_arduino shall implement the interface provided in the `platfom.h`.
```c
    MOCKABLE_FUNCTION(, int, platform_init);
    MOCKABLE_FUNCTION(, void, platform_deinit);
    MOCKABLE_FUNCTION(, const IO_INTERFACE_DESCRIPTION*, platform_get_default_tlsio);
```
**]**

**SRS_PLATFORM_ARDUINO_21_002: [** The platform_arduino shall use the tlsio functions defined by the 'xio.h'.
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

###  platform_init

```c
int platform_init(void)
```

**SRS_PLATFORM_ARDUINO_21_003: [** The platform_init shall initialize the platform. **]**
**SRS_PLATFORM_ARDUINO_21_004: [** The platform_init shall allocate any memory needed to control the platform. **]** 


###  platform_deinit

```c
int platform_deinit(void)
```

**SRS_PLATFORM_ARDUINO_21_005: [** The platform_deinit shall deinitialize the platform. **]**
**SRS_PLATFORM_ARDUINO_21_006: [** The platform_deinit shall free all allocate memory needed to control the platform. **]** 


###  platform_get_default_tlsio

```c
const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
```

**SRS_PLATFORM_ARDUINO_21_007: [** The platform_get_default_tlsio shall return a set of tlsio functions provided by the Arduino tlsio implementation. **]**
