lock requirements
================


## Overview

The Lock component is implemented to be able to achieve thread synchronization, as we may have a requirement to consume locks across different platforms .This component will expose some generic APIs so that it can be extended to platform specific implementation .

## Exposed API
**SRS_LOCK_10_001: [** `Lock` interface exposes the following APIs **]**
```c
typedef enum LOCK_RESULT_TAG
{
    LOCK_OK,
    LOCK_ERROR
} LOCK_RESULT;
```

```c
typedef void* HANDLE_LOCK;
```
**SRS_LOCK_10_015: [** This is the handle to the different lock instances **]**

```c
HANDLE_LOCK Lock_Init (void);
```
**SRS_LOCK_10_002: [** `Lock_Init` on success shall return a valid lock handle which should be a non `NULL` value **]**

**SRS_LOCK_10_003: [** `Lock_Init` on error shall return `NULL` **]**

```c
LOCK_RESULT Lock(HANDLE_LOCK handle);
```
**SRS_LOCK_10_004: [** `Lock` shall be implemented as a non-recursive lock on specific platforms **]**

**SRS_LOCK_10_005: [** `Lock` on success shall return `LOCK_OK` **]**

**SRS_LOCK_10_006: [** `Lock` on error shall return `LOCK_ERROR` **]**

**SRS_LOCK_10_007: [** `Lock` on `NULL` handle passed returns `LOCK_ERROR` **]**

```c
LOCK_RESULT Unlock(HANDLE_LOCK handle);
```
**SRS_LOCK_10_008: [** `Unlock` shall be implemented as a mutex unlock on specific platforms **]**

**SRS_LOCK_10_009: [** `Unlock` on success shall return `LOCK_OK` **]**

**SRS_LOCK_10_010: [** `Unlock` on error shall return `LOCK_ERROR` **]**

**SRS_LOCK_10_011: [** `Unlock` on `NULL` handle passed returns `LOCK_ERROR` **]**

```c
LOCK_RESULT Lock_Deinit(HANDLE_LOCK handle);
```
**SRS_LOCK_10_012: [** `Lock_Deinit` frees the memory pointed by handle **]**

**SRS_LOCK_10_013: [** `Lock_Deinit` on `NULL` handle passed returns `LOCK_ERROR` **]**
