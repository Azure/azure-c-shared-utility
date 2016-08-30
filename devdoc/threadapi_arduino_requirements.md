threadapi_arduino
=================

## Overview

threadapi_arduino implements a wrapper function for Arduino's delay function. Arduino do not support thread, so it returns error for the other ThreadAPI functions.

## References

[Delay](https://www.arduino.cc/en/Reference/Delay)

###  Exposed API

**SRS_THREADAPI_ARDUINO_21_001: [** The threadapi_arduino shall implement the method sleep defined by the `threadapi.h`. 
```c
/**
 * @brief	Sleeps the current thread for the given number of milliseconds.
 *
 * @param	milliseconds	The number of milliseconds to sleep.
 */
MOCKABLE_FUNCTION(, void, ThreadAPI_Sleep, unsigned int, milliseconds);
```
**]**


###  ThreadAPI_Sleep

```c
void ThreadAPI_Sleep(unsigned int milliseconds);
```

**SRS_THREADAPI_ARDUINO_21_002: [** The ThreadAPI_Sleep shall receive a time in milliseconds. **]**
**SRS_THREADAPI_ARDUINO_21_003: [** The ThreadAPI_Sleep shall stop the thread for the specified time. **]**


###  ThreadAPI_Create

```c
THREADAPI_RESULT ThreadAPI_Create(THREAD_HANDLE* threadHandle,  THREAD_START_FUNC func, void* arg);
```

**SRS_THREADAPI_ARDUINO_21_004: [** The Arduino do not support ThreadAPI_Create, it shall return THREADAPI_ERROR. **]**


###  ThreadAPI_Join

```c
THREADAPI_RESULT ThreadAPI_Join(THREAD_HANDLE threadHandle, int* res);
```

**SRS_THREADAPI_ARDUINO_21_005: [** The Arduino do not support ThreadAPI_Join, it shall return THREADAPI_ERROR. **]**


###  ThreadAPI_Exit

```c
void ThreadAPI_Exit(int res);
```

**SRS_THREADAPI_ARDUINO_21_006: [** The Arduino do not support ThreadAPI_Exit, it shall not do anything. **]**
