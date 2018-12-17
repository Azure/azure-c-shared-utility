# constbuffer_array_requirements
================

## Overview

`constbuffer_array_array` is a module that stiches several `CONSTBUFFER_ARRAY`s together. `constbuffer_array_array` can add/remove a `CONSTBUFFER_ARRAY` at the beginning (front) of the already constructed stitch. 

`CONSTBUFFER_ARRAY_ARRAY_HANDLE`s are immutable, that is, adding/removing a `CONSTBUFFER_ARRAY` to/from an existing `CONSTBUFFER_ARRAY_ARRAY` will result in a new `CONSTBUFFER_ARRAY_HANDLE`.

## Exposed API

```c
typedef struct CONSTBUFFER_ARRAY_ARRAY_HANDLE_DATA_TAG* CONSTBUFFER_ARRAY_ARRAY_HANDLE;

/*create*/
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_create, const CONSTBUFFER_ARRAY_HANDLE*, arrays, uint32_t, array_count);
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_create_empty);

MOCKABLE_FUNCTION(, void, constbuffer_array_array_inc_ref, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_handle);
MOCKABLE_FUNCTION(, void, constbuffer_array_array_dec_ref, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_handle);

/*add in front*/
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_add_front, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle);

/*remove front*/
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_remove_front, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle, CONSTBUFFER_ARRAY_HANDLE *, constbuffer_array_handle);

/* getters */
MOCKABLE_FUNCTION(, int, constbuffer_array_array_get_array_count, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle, uint32_t*, array_count);
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_array_get_array, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle, uint32_t, array_index);
```

### constbuffer_array_create

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_create, const CONSTBUFFER_ARRAY_HANDLE*, arrays, uint32_t, array_count);
```

`constbuffer_array_array_create` creates a new const buffer array array made of the constbuffer arrays in `arrays`.

**SRS_CONSTBUFFER_ARRAY_01_009: [** `constbuffer_array_array_create` shall allocate memory for a new `CONSTBUFFER_ARRAY_ARRAY_HANDLE` that can hold `array_count` `CONSTBUFFER_ARRAY_HANDLE`. **]**

**SRS_CONSTBUFFER_ARRAY_01_010: [** `constbuffer_array_array_create` shall call  `constbuffer_array_inc_ref` on each `CONSTBUFFER_ARRAY_HANDLE` and store it. **]**

**SRS_CONSTBUFFER_ARRAY_01_011: [** On success `constbuffer_array_array_create` shall return a non-NULL handle. **]**

**SRS_CONSTBUFFER_ARRAY_01_012: [** If `arrays` is NULL and `array_count` is not 0, `constbuffer_array_array_create` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_01_014: [** If any error occurs, `constbuffer_array_array_create` shall fail and return NULL. **]**

### constbuffer_array_create_empty

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_create_empty);
```

`constbuffer_array_array_create_empty` creates a new, empty CONSTBUFFER_ARRAY_ARRAY_HANDLE.

**SRS_CONSTBUFFER_ARRAY_02_004: [** `constbuffer_array_array_create_empty` shall allocate memory for a new `CONSTBUFFER_ARRAY_ARRAY_HANDLE`. **]**

**SRS_CONSTBUFFER_ARRAY_02_041: [** `constbuffer_array_array_create_empty` shall succeed and return a non-`NULL` value. **]**

**SRS_CONSTBUFFER_ARRAY_02_001: [** If are any failure is encountered, `constbuffer_array_array_create_empty` shall fail and return `NULL`. **]**

### constbuffer_array_inc_ref

```c
MOCKABLE_FUNCTION(, void, constbuffer_array_array_inc_ref, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle);
```

`constbuffer_array_inc_ref` increments the reference count for `constbuffer_array_array_handle`.

**SRS_CONSTBUFFER_ARRAY_01_017: [** If `constbuffer_array_array_handle` is `NULL` then `constbuffer_array_array_inc_ref` shall return. **]**

**SRS_CONSTBUFFER_ARRAY_01_018: [** Otherwise `constbuffer_array_array_inc_ref` shall increment the reference count for `constbuffer_array_array_handle`. **]**

### constbuffer_array_dec_ref

```c
MOCKABLE_FUNCTION(, void, constbuffer_array_array_dec_ref, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle);
```

`constbuffer_array_array_dec_ref` decrements the reference count and frees all used resources if needed.

**SRS_CONSTBUFFER_ARRAY_02_039: [** If `constbuffer_array_array_handle` is `NULL` then `constbuffer_array_array_dec_ref` shall return. **]**

**SRS_CONSTBUFFER_ARRAY_01_016: [** Otherwise `constbuffer_array_array_dec_ref` shall decrement the reference count for `constbuffer_array_array_handle`. **]**

**SRS_CONSTBUFFER_ARRAY_02_038: [** If the reference count reaches 0, `constbuffer_array_array_dec_ref` shall free all used resources. **]**

### constbuffer_array_add_front

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_add_front, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_handle);
```

`constbuffer_array_array_add_front` adds a new `CONSTBUFFER_ARRAY_HANDLE` at the front of the already stored `CONSTBUFFER_ARRAY_HANDLE`s.

**SRS_CONSTBUFFER_ARRAY_02_006: [** If `constbuffer_array_array_handle` is `NULL` then `constbuffer_array_array_add_front` shall fail and return `NULL` **]**

**SRS_CONSTBUFFER_ARRAY_02_007: [** If `constbuffer_array_handle` is `NULL` then `constbuffer_array_array_add_front` shall fail and return `NULL` **]**

**SRS_CONSTBUFFER_ARRAY_02_042: [** `constbuffer_array_array_add_front` shall allocate enough memory to hold all of `constbuffer_array_array_handle` existing `CONSTBUFFER_ARRAY_HANDLE` and `constbuffer_array_handle`. **]**

**SRS_CONSTBUFFER_ARRAY_02_043: [** `constbuffer_array_array_add_front` shall copy `constbuffer_array_handle` and all of `constbuffer_array_array_handle` existing `CONSTBUFFER_ARRAY_HANDLE`. **]**

**SRS_CONSTBUFFER_ARRAY_02_044: [** `constbuffer_array_array_add_front` shall inc_ref all the `CONSTBUFFER_ARRAY_ARRAY_HANDLE` it had copied. **]**

**SRS_CONSTBUFFER_ARRAY_02_010: [** `constbuffer_array_array_add_front` shall succeed and return a non-`NULL` value. **]**

**SRS_CONSTBUFFER_ARRAY_02_011: [** If there any failures `constbuffer_array_array_add_front` shall fail and return `NULL`. **]**

### constbuffer_array_remove_front

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_remove_front, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle, CONSTBUFFER_ARRAY_HANDLE *, constbuffer_array_handle);
```

`constbuffer_array_array_remove_front` removes the front `CONSTBUFFER_ARRAY_HANDLE` and hands it over to the caller.

**SRS_CONSTBUFFER_ARRAY_02_012: [** If `constbuffer_array_array_handle` is `NULL` then `constbuffer_array_array_remove_front` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_02_045: [** If `constbuffer_array_handle` is `NULL` then `constbuffer_array_array_remove_front` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_02_013: [** If there is no front `CONSTBUFFER_ARRAY_HANDLE` then `constbuffer_array_array_remove_front` shall fail and return `NULL`. **]**

**SRS_CONSTBUFFER_ARRAY_02_002: [** `constbuffer_array_array_remove_front` shall fail when called on a newly constructed `CONSTBUFFER_ARRAY_ARRAY_HANDLE`. **]**

**SRS_CONSTBUFFER_ARRAY_02_046: [** `constbuffer_array_array_remove_front` shall allocate memory to hold all of `constbuffer_array_array_handle` `CONSTBUFFER_ARRAY_HANDLE`s except the front one. **]**

**SRS_CONSTBUFFER_ARRAY_02_047: [** `constbuffer_array_array_remove_front` shall copy all of `constbuffer_array_array_handle` `CONSTBUFFER_ARRAY_HANDLE`s except the front one. **]**

**SRS_CONSTBUFFER_ARRAY_02_048: [** `constbuffer_array_array_remove_front` shall inc_ref all the copied `CONSTBUFFER_ARRAY_HANDLE`s. **]**

**SRS_CONSTBUFFER_ARRAY_01_001: [** `constbuffer_array_array_remove_front` shall inc_ref the removed buffer. **]**

**SRS_CONSTBUFFER_ARRAY_02_049: [** `constbuffer_array_array_remove_front` shall succeed and return a non-`NULL` value. **]**

**SRS_CONSTBUFFER_ARRAY_02_036: [** If there are any failures then `constbuffer_array_array_remove_front` shall fail and return `NULL`. **]**

### constbuffer_array_get_buffer_count

```c
MOCKABLE_FUNCTION(, int, constbuffer_array_array_get_array_count, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle, uint32_t*, array_count);
```

`constbuffer_array_array_get_array_count` gets the count of constbuffer_arrays held by the constbuffer_array_array.

**SRS_CONSTBUFFER_ARRAY_01_002: [** On success, `constbuffer_array_array_get_array_count` shall return 0 and write the array count in `array_count`. **]**

**SRS_CONSTBUFFER_ARRAY_01_003: [** If `constbuffer_array_array_handle` is NULL, `constbuffer_array_array_get_array_count` shall fail and return a non-zero value. **]**

**SRS_CONSTBUFFER_ARRAY_01_004: [** If `array_count` is NULL, `constbuffer_array_array_get_array_count` shall fail and return a non-zero value. **]**

### constbuffer_array_get_buffer

```c
MOCKABLE_FUNCTION(, CONSTBUFFER_ARRAY_HANDLE, constbuffer_array_array_get_array, CONSTBUFFER_ARRAY_ARRAY_HANDLE, constbuffer_array_array_handle, uint32_t, array_index);
```

`constbuffer_array_array_get_array` returns the constbuffer_array at the `array_index`-th given index in the array.

**SRS_CONSTBUFFER_ARRAY_01_005: [** On success, `constbuffer_array_array_get_array` shall return a non-NULL handle to the `array_index`-th const buffer in the array. **]**

**SRS_CONSTBUFFER_ARRAY_01_006: [** The returned handle shall have its reference count incremented. **]**

**SRS_CONSTBUFFER_ARRAY_01_007: [** If `constbuffer_array_array_handle` is NULL, `constbuffer_array_array_get_array` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_01_008: [** If `array_index` is greater or equal to the number of buffers in the array, `constbuffer_array_array_handle` shall fail and return NULL. **]**

**SRS_CONSTBUFFER_ARRAY_01_015: [** If any error occurs, `constbuffer_array_array_handle` shall fail and return NULL. **]**

### constbuffer_array_get_buffer_content

