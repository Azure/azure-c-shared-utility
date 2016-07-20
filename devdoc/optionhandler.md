# OptionHandler

# Overview 

Option Handler is a module that builds a container of options relevant to a module. The options can be later retrieved. 
It does so by asking the module to clone its options or to destroy them. `OptionHandler` is agnostic to the module it serves.


## Exposed API

```c
#define OPTIONHANDLER_RESULT_VALUES
OPTIONHANDLER_OK,
OPTIONHANDLER_ERROR,
OPTIONHANDLER_INVALIDARG

DEFINE_ENUM(OPTIONHANDLER_RESULT, OPTIONHANDLER_RESULT_VALUES)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "azure_c_shared_utility/umock_c_prod.h"

typedef struct OPTIONHANDLER_HANDLE_DATA_TAG* OPTIONHANDLER_HANDLE;

/*the following function pointer points to a function that produces a clone of the option specified by name and value (that is, a clone of void* value)*/
/*returns NULL if it failed to produce a clone, otherwise returns a non-NULL value*/
/*to be implemented by every module*/
typedef void* (*pfCloneOption)(const char* name, void* value);

/*the following function pointer points to a function that frees resources allocated for an option*/
/*to be implemented by every module*/
typedef void (*pfDestroyOption)(const char* name, void* value);

/*the following function pointer points to a function that sets an option for a module*/
/*to be implemented by every module*/
typedef int (*pfSetOption)(void* handle, const char* name, void* value);

MOCKABLE_FUNCTION(,OPTIONHANDLER_HANDLE, OptionHandler_Create, pfCloneOption, cloneOption, pfDestroyOption, destroyOption, pfSetOption setOption);
MOCKABLE_FUNCTION(,OPTIONHANDLER_RESULT, OptionHandler_AddOption, OPTIONHANDLER_HANDLE, handle, const char*, name, void*, value);
MOCKABLE_FUNCTION(,OPTIONHANDLER_RESULT, OptionHandler_FeedOptions, OPTIONHANDLER_HANDLE, handle, void*, destinationHandle);
MOCKABLE_FUNCTION(,void, OptionHandler_Destroy, OPTIONHANDLER_HANDLE, handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

### OptionHandler_Create
```c
OPTIONHANDLER_HANDLE OptionHandler_Create(pfCloneOption cloneOption, pfDestroyOption destroyOption, pfSetOption setOption)
```

**SRS_OPTIONHANDLER_02_001: [** `OptionHandler_Create` shall fail and retun `NULL` if any parameters are `NULL`. **]**
**SRS_OPTIONHANDLER_02_002: [** `OptionHandler_Create` shall create an empty VECTOR that will hold pairs of `const char*` and `void*`. **]**
**SRS_OPTIONHANDLER_02_003: [** If all the operations succeed then `OptionHandler_Create` shall succeed and return a non-`NULL` handle. **]**
**SRS_OPTIONHANDLER_02_004: [** Otherwise, `OptionHandler_Create` shall fail and return `NULL`. **]**

### OptionHandler_AddOption
```c
OPTIONHANDLER_RESULT OptionHandler_AddOption(OPTIONHANDLER_HANDLE handle, const char* name, void* value)
```

**SRS_OPTIONHANDLER_02_005: [** `OptionHandler_AddOption` shall fail and return `OPTIONHANDLER_INVALIDARG` if any parameter is NULL. **]**
**SRS_OPTIONHANDLER_02_006: [** OptionHandler_AddOption shall call `pfCloneOption` passing `name` and `value`. **]**
**SRS_OPTIONHANDLER_02_007: [** OptionHandler_AddOption shall use `VECTOR` APIs to save the `name` and the newly created clone of `value`. **]**
**SRS_OPTIONHANDLER_02_008: [** If all the operations succed then `OptionHandler_AddOption` shall succeed and return `OPTIONHANDLER_OK`. **]**
**SRS_OPTIONHANDLER_02_009: [** Otherwise, `OptionHandler_AddOption` shall succeed and return `OPTIONHANDLER_ERROR`. **]**

### OptionHandler_FeedOptions
```c
OPTIONHANDLER_RESULT OptionHandler_FeedOptions(OPTIONHANDLER_HANDLE handle, void* destinationHandle);
```

**SRS_OPTIONHANDLER_02_010: [** `OptionHandler_FeedOptions` shall fail and return `OPTIONHANDLER_INVALIDARG` if any argument is `NULL`. **]**
**SRS_OPTIONHANDLER_02_011: [** Otherwise, `OptionHandler_FeedOptions` shall use `VECTOR`'s iteration mechanisms to retrieve pairs of name, value (`const char*` and `void*`). **]**
**SRS_OPTIONHANDLER_02_012: [** `OptionHandler_FeedOptions` shall call for every pair of name,value `setOption` passing `destinationHandle`, name and value. **]**
**SRS_OPTIONHANDLER_02_013: [** If all the operations succeed then `OptionHandler_FeedOptions` shall succeed and return `OPTIONHANDLER_OK`. **]**
**SRS_OPTIONHANDLER_02_014: [** Otherwise, `OptionHandler_FeedOptions` shall fail and return `OPTIONHANDLER_ERROR`. **]**

### OptionHandler_Destroy
```c
void OptionHandler_Destroy(OPTIONHANDLER_HANDLE handle)
```
**SRS_OPTIONHANDLER_02_015: [** OptionHandler_Destroy shall do nothing if parameter `handle` is `NULL`. **]**
**SRS_OPTIONHANDLER_02_016: [** Otherwise, OptionHandler_Destroy shall free all used resources. **]**
