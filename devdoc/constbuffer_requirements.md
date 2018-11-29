ConstBuffer Requirements
================


## Overview

ConstBuffer is a module that implements a read-only buffer of bytes (unsigned char). 
Once created, the buffer can no longer be changed. The buffer is ref counted so further _Clone calls result in
zero copy.


## References
[refcount](../inc/refcount.h)

[buffer](buffer_requirements.md)

## Exposed API
```c
/*this is the handle*/
typedef struct CONSTBUFFER_HANDLE_DATA_TAG* CONSTBUFFER_HANDLE;

/*this is what is returned when the content of the buffer needs access*/
typedef struct CONSTBUFFER_TAG
{
    const unsigned char* buffer;
    size_t size;
} CONSTBUFFER;

/*this creates a new constbuffer from a memory area*/
MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_Create, const unsigned char*, source, size_t, size);

/*this creates a new constbuffer from an existing BUFFER_HANDLE*/
MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_CreateFromBuffer, BUFFER_HANDLE, buffer);

MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_CreateWithMoveMemory, unsigned char*, source, size_t, size);

MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_Clone, CONSTBUFFER_HANDLE, constbufferHandle);

MOCKABLE_FUNCTION(, const CONSTBUFFER*, CONSTBUFFER_GetContent, CONSTBUFFER_HANDLE, constbufferHandle);

MOCKABLE_FUNCTION(, void, CONSTBUFFER_Destroy, CONSTBUFFER_HANDLE, constbufferHandle);
```

###  CONSTBUFFER_Create
```c
MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_Create, const unsigned char*, source, size_t, size);
```
**SRS_CONSTBUFFER_02_001: [** If `source` is NULL and `size` is different than 0 then CONSTBUFFER_Create shall fail and return NULL. **]**

**SRS_CONSTBUFFER_02_002: [** Otherwise, `CONSTBUFFER_Create` shall create a copy of the memory area pointed to by `source` having `size` bytes. **]**

**SRS_CONSTBUFFER_02_003: [** If creating the copy fails then `CONSTBUFFER_Create` shall return NULL. **]**

**SRS_CONSTBUFFER_02_004: [** Otherwise `CONSTBUFFER_Create` shall return a non-NULL handle. **]**

**SRS_CONSTBUFFER_02_005: [** The non-NULL handle returned by `CONSTBUFFER_Create` shall have its ref count set to "1". **]** 

### CONSTBUFFER_CreateFromBuffer
```c
MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_CreateFromBuffer, BUFFER_HANDLE, buffer);
```
**SRS_CONSTBUFFER_02_006: [** If `buffer` is NULL then `CONSTBUFFER_CreateFromBuffer` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_02_007: [** Otherwise, `CONSTBUFFER_CreateFromBuffer` shall copy the content of `buffer`. **]**

**SRS_CONSTBUFFER_02_008: [** If copying the content fails, then `CONSTBUFFER_CreateFromBuffer` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_02_009: [** Otherwise, `CONSTBUFFER_CreateFromBuffer` shall return a non-NULL handle. **]**

**SRS_CONSTBUFFER_02_010: [** The non-NULL handle returned by `CONSTBUFFER_CreateFromBuffer` shall have its ref count set to "1". **]** 

### CONSTBUFFER_CreateWithMoveMemory
```c
MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_CreateWithMoveMemory, unsigned char*, source, size_t, size);
```

`CONSTBUFFER_CreateWithMoveMemory` creates a CONST buffer with move semantics for the memory given as argument (if succesfull, the const buffer owns the memory from that point on).
The memory is assumed to be freeable by a call to `free`.

**SRS_CONSTBUFFER_01_001: [** If `source` is NULL and `size` is different than 0 then `CONSTBUFFER_CreateWithMoveMemory` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_01_004: [** If `source` is non-NULL and `size` is 0, the `source` pointer shall be owned (and freed) by the newly created instance of const buffer. **]**

**SRS_CONSTBUFFER_01_002: [** Otherwise, `CONSTBUFFER_CreateWithMoveMemory` shall store the `source` and `size` and return a non-NULL handle to the newly created const buffer. **]**

**SRS_CONSTBUFFER_01_003: [** The non-NULL handle returned by `CONSTBUFFER_CreateWithMoveMemory` shall have its ref count set to "1". **]**

### CONSTBUFFER_Clone
```c
MOCKABLE_FUNCTION(, CONSTBUFFER_HANDLE, CONSTBUFFER_Clone, CONSTBUFFER_HANDLE, constbufferHandle);
```
**SRS_CONSTBUFFER_02_013: [** If `constbufferHandle` is NULL then CONSTBUFFER_Clone shall fail and return NULL. **]**

**SRS_CONSTBUFFER_02_014: [** Otherwise, `CONSTBUFFER_Clone` shall increment the reference count and return `constbufferHandle`. **]**

### CONSTBUFFER_GetContent
```c
MOCKABLE_FUNCTION(, const CONSTBUFFER*, CONSTBUFFER_GetContent, CONSTBUFFER_HANDLE, constbufferHandle);
```
**SRS_CONSTBUFFER_02_011: [** If `constbufferHandle` is NULL then CONSTBUFFER_GetContent shall return NULL. **]**

**SRS_CONSTBUFFER_02_012: [** Otherwise, `CONSTBUFFER_GetContent` shall return a `const CONSTBUFFER*` that matches byte by byte the original bytes used to created the const buffer and has the same length. **]**

### CONSTBUFFER_Destroy
```c
MOCKABLE_FUNCTION(, void, CONSTBUFFER_Destroy, CONSTBUFFER_HANDLE, constbufferHandle);
```
**SRS_CONSTBUFFER_02_015: [** If `constbufferHandle` is NULL then `CONSTBUFFER_Destroy` shall do nothing. **]**

**SRS_CONSTBUFFER_02_016: [** Otherwise, `CONSTBUFFER_Destroy` shall decrement the refcount on the `constbufferHandle` handle. **]**

**SRS_CONSTBUFFER_02_017: [** If the refcount reaches zero, then `CONSTBUFFER_Destroy` shall deallocate all resources used by the CONSTBUFFER_HANDLE. **]**




