#umockcall requirements
 
#Overview

umockcall is a module that encapsulates a umock call.

#Exposed API

```c
    typedef struct UMOCKCALL_TAG* UMOCKCALL_HANDLE;
    typedef int(*UMOCKCALL_DATA_CLONE_FUNC)(void* destination, const void* source);
    typedef void(*UMOCKCALL_DATA_FREE_FUNC)(void* umockcall_data);
    typedef char*(*UMOCKCALL_DATA_STRINGIFY_FUNC)(void* umockcall_data);
    typedef int(*UMOCKCALL_DATA_ARE_EQUAL_FUNC)(void* left, void* right);

    extern UMOCKCALL_HANDLE umockcall_create(const char* function_name, void* umockcall_data, void* set_return_func, void* ignore_all_arguments_func, UMOCKCALL_DATA_CLONE_FUNC umockcall_data_clone, UMOCKCALL_DATA_FREE_FUNC umockcall_data_free, UMOCKCALL_DATA_STRINGIFY_FUNC umockcall_data_stringify, UMOCKCALL_DATA_ARE_EQUAL_FUNC umockcall_data_are_equal);
    extern void umockcall_destroy(UMOCKCALL_HANDLE umockcall);
    extern int umockcall_are_equal(UMOCKCALL_HANDLE left, UMOCKCALL_HANDLE right);
    extern char* umockcall_to_string(UMOCKCALL_HANDLE umockcall);
    extern void* umockcall_get_call_data(UMOCKCALL_HANDLE umockcall);
```

##umockcall_create

```c
extern UMOCKCALL_HANDLE umockcall_create(const char* function_name, void* umockcall_data, void* set_return_func, void* ignore_all_arguments_func, UMOCKCALL_DATA_CLONE_FUNC umockcall_data_clone, UMOCKCALL_DATA_FREE_FUNC umockcall_data_free, UMOCKCALL_DATA_STRINGIFY_FUNC umockcall_data_stringify, UMOCKCALL_DATA_ARE_EQUAL_FUNC umockcall_data_are_equal);
```

**SRS_UMOCKCALL_01_001: [** umockcall_create shall create a new instance of a umock call and on success it shall return a non-NULL handle to it. **]**
**SRS_UMOCKCALL_01_002: [** If allocating memory for the umock call instance fails, umockcall_create shall return NULL. **]**
**SRS_UMOCKCALL_01_003: [** If any of the arguments are NULL, umockcall_create shall fail and return NULL. **]**   

##umockcall_destroy

```c
extern void umockcall_destroy(UMOCKCALL_HANDLE umockcall);
```

**SRS_UMOCKCALL_01_004: [** umockcall_destroy shall free a previously allocated umock call instance. **]**
**SRS_UMOCKCALL_01_005: [** If the umockcall argument is NULL then umockcall_destroy shall do nothing. **]**   

##umockcall_are_equal

```c
extern int umockcall_are_equal(UMOCKCALL_HANDLE left, UMOCKCALL_HANDLE right);
```

**SRS_UMOCKCALL_01_006: [** umockcall_are_equal shall compare the two mock calls and return whether they are equal or not. **]**
**SRS_UMOCKCALL_01_007: [** If the 2 mock calls are equal umockcall_are_equal shall return 1. **]**
**SRS_UMOCKCALL_01_008: [** If the 2 mock calls are not equal umockcall_are_equal shall return 0. **]**
**SRS_UMOCKCALL_01_009: [** If any error occurred during the comparison umockcall_are_equal shall return -1. **]**
**SRS_UMOCKCALL_01_012: [** For two calls to be equal the following shall be equal: **]**
**SRS_UMOCKCALL_01_010: [** - The function name **]**
**SRS_UMOCKCALL_01_011: [** - The call data **]**
**SRS_UMOCKCALL_01_013: [** The call data shall be compared by calling the function passed to umockcall_create. **]**
**SRS_UMOCKCALL_01_014: [** If the two calls have different are_equal functions that have been passed to umockcall_create then teh calls shall be considered different. **]**

##umockcall_to_string

```c
extern char* umockcall_to_string(UMOCKCALL_HANDLE umockcall);
```