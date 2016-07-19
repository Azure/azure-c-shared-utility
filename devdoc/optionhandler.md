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

#define SETOPTION_RESULT_VALUES
SETOPTION_OK, /*returned when the option has been set*/
SETOPTION_ERROR, /*returned when the option has NOT been set*/ 
SETOPTION_INVALIDARG /*returned when arguments are invalid (such as NULL arguments, or option names not handled by the  module*/

DEFINE_ENUM(SETOPTION_RESULT, SETOPTION_RESULT_VALUES)

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
typedef SETOPTION_RESULT (*pfSetOption)(void* handle, const char* name, void* value);

MOCKABLE_FUNCTION(,OPTIONHANDLER_HANDLE, OptionHandler_Create, pfCloneOption, cloneOption, pfDestroyOption, destroyOption, pfSetOption setOption);
MOCKABLE_FUNCTION(,OPTIONHANDLER_RESULT, OptionHandler_AddProperty, OPTIONHANDLER_HANDLE, handle, const char*, name, void*, value);
MOCKABLE_FUNCTION(,OPTIONHANDLER_RESULT, OptionHandler_FeedOptions, OPTIONHANDLER_HANDLE, handle, void*, destinationHandle);
MOCKABLE_FUNCTION(,void, OptionHandler_Destroy, OPTIONHANDLER_HANDLE, handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

### OptionHandler_Create
```c
OPTIONHANDLER_HANDLE OptionHandler_Create(pfCloneOption cloneOption, pfDestroyOption destroyOption, pfSetOption setOption)
```
`OptionHandler_Create` shall fail and retun `NULL` if any parameters are `NULL`.
`OptionHandler_Create` shall create an empty VECTOR that will hold pairs of `const char*` and `void*`.
If all the operations succeed then `OptionHandler_Create` shall succeed and return a non-`NULL` handle.
Otherwise, `OptionHandler_Create` shall fail and return `NULL`.

### OptionHandler_AddProperty
```c
OPTIONHANDLER_RESULT OptionHandler_AddProperty(OPTIONHANDLER_HANDLE handle, const char* name, void* value)
```
`OptionHandler_AddProperty` shall fail and return `OPTIONHANDLER_INVALIDARG` if any parameter is NULL.
OptionHandler_AddProperty shall call `pfCloneOption` passing `name` and `value`.
OptionHandler_AddProperty shall use `VECTOR` APIs to save the `name` and the newly created clone of `value`.
If all the operations succed then `OptionHandler_AddProperty` shall succeed and return `OPTIONHANDLER_OK`.
Otherwise, `OptionHandler_AddProperty` shall succeed and return `OPTIONHANDLER_ERROR`.

### OptionHandler_FeedOptions
```c
OPTIONHANDLER_RESULT OptionHandler_FeedOptions(OPTIONHANDLER_HANDLE handle, void* destinationHandle);
```
`OptionHandler_FeedOptions` shall fail and return `OPTIONHANDLER_INVALIDARG` if any argument is `NULL`.
Otherwise, `OptionHandler_FeedOptions` shall use `VECTOR`'s iteration mechanisms to retrieve pairs of name, value (`const char*` and `void*`).
`OptionHandler_FeedOptions` shall call for every pair of name,value `setOption` passing `destinationHandle`, name and value.
If all the operations succed then `OptionHandler_FeedOptions` shall succeed and return `OPTIONHANDLER_OK`.
Otherwise, `OptionHandler_FeedOptions` shall fail and return `OPTIONHANDLER_ERROR`.

### OptionHandler_Destroy
```c
void OptionHandler_Destroy(OPTIONHANDLER_HANDLE handle)
```
OptionHandler_Destroy shall do nothing is parameter `handle` is `NULL`.
Otherwise, OptionHandler_Destroy shall free all used resources.