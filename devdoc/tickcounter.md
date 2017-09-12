tickcounter
=========

## Overview

This document describes how to write a tickcounter adapter for Azure IoT.


## References
[Azure IoT C Shared Utility tickcounter adapter](https://github.com/Azure/azure-c-shared-utility/blob/master/doc/porting_guide.md#tickcounter-adapter)  
[tickcounter.h](https://github.com/Azure/azure-c-shared-utility/blob/master/inc/azure_c_shared_utility/tickcounter.h)



## Exposed API

**SRS_TICKCOUNTER_30_001: [** The tickcounter adapter shall use the following data types as defined in tickcounter.h.
```c
// uint_fast32_t is a 32 bit uint or larger
typedef uint_fast32_t tickcounter_ms_t;
// TICK_COUNTER_INSTANCE_TAG* is an opaque handle type
typedef struct TICK_COUNTER_INSTANCE_TAG* TICK_COUNTER_HANDLE;
```
 **]**  

**SRS_TICKCOUNTER_30_002: [** The tickcounter adapter shall implement the API calls defined in tickcounter.h:
```c
TICK_COUNTER_HANDLE tickcounter_create(void);
void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter);
int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t* current_ms);
```
 **]**  


###   tickcounter_create
The `tickcounter_create` call allocates and initializes an internally defined 
TICK_COUNTER_INSTANCE and returns its pointer as type TICK_COUNTER_HANDLE.

As a memory optimization, implementations using an underlying free-running 32-bit counter could 
avoid heap allocation by casting the initial counter value to the TICK_COUNTER_HANDLE type directly
and returning that value rather
than storing a struct on the heap. This would require changing setting any 0 counter values to 1 first,
but this is acceptable because tickcounter is not required to provide any particular minimum resolution.
```c
TICK_COUNTER_HANDLE tickcounter_create(void);
```

**SRS_TICKCOUNTER_30_003: [** `tickcounter_create` shall allocate and initialize an internally-defined TICK_COUNTER_INSTANCE structure and return its pointer on success. **]**

**SRS_TICKCOUNTER_30_004: [** If allocation of the internally-defined TICK_COUNTER_INSTANCE structure fails,  `tickcounter_create` shall return NULL. **]**  


###   tickcounter_destroy
The `tickcounter_destroy` call releases all resources acquired by the `tickcounter_create` call.
```c
void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter);
```

**SRS_TICKCOUNTER_30_005: [** `tickcounter_destroy` shall delete the internally-defined TICK_COUNTER_INSTANCE structure specified by the `tick_counter` parameter. (This call has no failure case.) **]**

**SRS_TICKCOUNTER_30_006: [** If the `tick_counter` parameter is NULL, `tickcounter_destroy` shall do nothing. **]**  


###   tickcounter_get_current_ms
The `tickcounter_get_current_ms` call returns the number of milleconds elapsed since the 
`tickcounter_create` call. Although the resolution of this value is milliseconds, tickcounter is used
by the SDK primarily for timeout calculations, so implementations are not required to have millisecond
count quantization; 1000 milliseconds is an acceptable count quantum, and no maximum count 
quantum is specified.

```c
int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t* current_ms);
```

**SRS_TICKCOUNTER_30_007: [** If the `tick_counter` parameter is NULL, `tickcounter_get_current_ms` shall return a non-zero value to indicate error. **]**

**SRS_TICKCOUNTER_30_008: [** If the `current_ms` parameter is NULL, `tickcounter_get_current_ms` shall return a non-zero value to indicate error. **]**

**SRS_TICKCOUNTER_30_009:  [** `tickcounter_get_current_ms` shall set `*current_ms` to the number of milliseconds elapsed since the `tickcounter_create` call for the specified `tick_counter` and return 0 to indicate success This call will rarely (if ever) have a failure case. **]**

**SRS_TICKCOUNTER_30_010: [** If the underlying counter (typically a free-running 32 bit counter)
experiences a single overflow between the calls to `tickcounter_create` and 
`tickcounter_get_current_ms`, the `tickcounter_get_current_ms` call shall 
still return the correct interval. For free-running 32 bit counters this means that the  
count value must be stored as an `int32_t` type rather than `uint_fast32_t` to avoid the 
possibility of underflow errors in case `uint_fast32_t` is implemented as 64 bits. **]**  
