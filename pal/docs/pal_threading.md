# pal_threading

## Overview

The **pal_threading** component is a part of the Azure IoT C SDK's [Physical Abstraction Layer](https://github.com/Azure/azure-c-shared-utility/tree/pal/pal) **TODO: link->master**.

The pal_os component exposes threading and synchronization functions to the Azure IoT C SDK. The pal_threading component is optional. 

Communicating with an Azure IoT Hub on a background thread will consume more memory than a single-threaded design because of the need for two stacks instead of one, so devices which are short on memory may prefer not to implement the pal_threading component. However, pal_dns components which use `getaddrinfo` or its equivalent (currently all implementations) may block repeatedly for a significant fraction of a minute if DNS servers are unavailable. So for the current SDK, devices which must always be responsive will require threading. 

## References

[pal_threading.h](https://github.com/Azure/azure-c-shared-utility/blob/pal/pal/include/pal_threading.h) **TODO: link->master** <br/>

## API

### threading types

```
typedef enum { LOCK_OK, LOCK_ERROR } LOCK_RESULT;
typedef struct PAL_LOCK_INSTANCE_TAG* LOCK_HANDLE;
typedef struct PAL_THREAD_INSTANCE_TAG* THREAD_HANDLE;
typedef int(*THREAD_START_FUNC)(void *);
```

### pal_lock_create
```
// Returns a lock handle that can be used in subsequent calls to lock/unlock.
LOCK_HANDLE pal_lock_create();
```

Although the pal_lock_create function implies unlimited heap-based allocation, the SDK really only uses a very small and predictable number of locks. So the PAL implementer or application developer may prefer to design a more efficient custom lock allocation scheme.

### pal_lock_destroy

```
void pal_lock_destroy(LOCK_HANDLE lock);
```
Release the specified lock.

### pal_get_time
```
time_t pal_get_time(time_t* p);
```
This is a device-independent version of the C `time()` function. The Azure IoT C SDK uses it for cert validity checking and for reporting date-time values. The PAL is agnostic about whether this is local time or UTC.

### pal_lock
```
LOCK_RESULT pal_lock(LOCK_HANDLE lock);
```
Acquire the specified lock. Returns 0 on success.

### pal_unlock
```
LOCK_RESULT pal_unlock(LOCK_HANDLE lock);
```
Acquire the specified lock. Returns 0 on success.

### pal_thread_create
```
THREAD_HANDLE pal_thread_create(THREAD_START_FUNC func, void * arg);
```
Create a thread with the specified entry point and argument. Returns NULL on failure.

### pal_thread_join
```
int pal_thread_join(THREAD_HANDLE thread_handle, int* exit_code);
```
Wait for the specified thread to exit, and optionally receive its result in exit_code. The exit_code parameter may be NULL. Returns 0 on success.

### pal_thread_exit
```
void pal_thread_exit(int exit_code);
```
Called by the created thread when exiting. The supplied `exit_code` will be passed to `pal_thread_join`.
