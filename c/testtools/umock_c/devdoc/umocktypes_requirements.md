#umocktypes requirements
 
#Overview

umocktypes is a module that exposes a generic type interface to be used by umockc for registering various C types. Later operations are possible with these types, specifically: converting any type value to a string, comparing values, copying and freeing values.

#Exposed API

```c
typedef char*(*UMOCKTYPE_STRINGIFY_FUNC)(const void* value);
typedef int(*UMOCKTYPE_COPY_FUNC)(void* destination, const void* source);
typedef void(*UMOCKTYPE_FREE_FUNC)(void* value);
typedef int(*UMOCKTYPE_ARE_EQUAL_FUNC)(const void* left, const void* right);

extern int umocktypes_init(void);
extern void umocktypes_deinit(void);
extern int umocktypes_register_type(const char* type, UMOCKTYPE_STRINGIFY_FUNC stringify, UMOCKTYPE_ARE_EQUAL_FUNC are_equal, UMOCKTYPE_COPY_FUNC value_copy, UMOCKTYPE_FREE_FUNC value_free);

extern char* umocktypes_stringify(const char* type, const void* value);
extern int umocktypes_are_equal(const char* type, const void* left, const void* right);
extern int umocktypes_copy(const char* type, void* destination, const void* source);
extern void umocktypes_free(const char* type, void* value);
```

##umocktypes_init

```c
extern int umocktypes_init(void);
```

**SRS_UMOCKTYPES_01_001: [** umocktypes_init shall initialize the umocktypes module. **]**
**SRS_UMOCKTYPES_01_002: [** After initialization the list of registered type shall be empty. **]**
**SRS_UMOCKTYPES_01_003: [** On success umocktypes_init shall return 0. **]**
**SRS_UMOCKTYPES_01_004: [** umocktypes_init after another umocktypes_init without deinitializing the module shall fail and return a non-zero value. **]**

##umocktypes_deinit

```c
extern void umocktypes_deinit(void);
```

**SRS_UMOCKTYPES_01_005: [** umocktypes_deinit shall free all resources associated with the registered types and shall leave the module in a state where another init is possible. **]**
**SRS_UMOCKTYPES_01_006: [** If the module was not initialized, umocktypes_deinit shall do nothing. **]**

