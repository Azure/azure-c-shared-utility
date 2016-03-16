#umocktypes_c requirements
 
#Overview

umocktypes_c is a module that exposes out of the box functionality for most C supported types. This includes integers, floats, size_t, etc.

#Exposed API

```c
extern int umocktypes_c_register_types(void);

#define UMOCKTYPES_HANDLERS(type, function_postfix) \
    extern char* C2(umocktypes_stringify_,function_postfix)(const type* value); \
    extern int C2(umocktypes_are_equal_, function_postfix)(const type* left, const type* right); \
    extern int C2(umocktypes_copy_, function_postfix)(type* destination, const type* source); \
    extern void C2(umocktypes_free_, function_postfix)(type* value);

UMOCKTYPES_HANDLERS(char, char)
UMOCKTYPES_HANDLERS(unsigned char, unsignedchar)
UMOCKTYPES_HANDLERS(short, short)
UMOCKTYPES_HANDLERS(unsigned short, unsignedshort)
UMOCKTYPES_HANDLERS(int, int)
UMOCKTYPES_HANDLERS(unsigned int, unsignedint)
UMOCKTYPES_HANDLERS(long, long)
UMOCKTYPES_HANDLERS(unsigned long, unsignedlong)
UMOCKTYPES_HANDLERS(long long, longlong)
UMOCKTYPES_HANDLERS(unsigned long long, unsignedlonglong)

```

##umocktypes_charptr_register_types

```c
extern int umocktypes_c_register_types(void);
```

**SRS_UMOCKTYPES_C_01_001: [** umocktypes_c_register_types shall register support for all the types in the module. **]**

##umocktypes_stringify_int

```c
extern char* umocktypes_stringify_int(const int* value);
```

**SRS_UMOCKTYPES_C_01_002: [** umocktypes_stringify_int shall return the string representation of value. **]**
**SRS_UMOCKTYPES_C_01_003: [** If value is NULL, umocktypes_stringify_int shall return NULL. **]**
**SRS_UMOCKTYPES_C_01_004: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_int shall return NULL. **]**
**SRS_UMOCKTYPES_C_01_005: [** If any other error occurs when creating the string representation, umocktypes_stringify_int shall return NULL. **]**

##umocktypes_are_equal_int

```c
extern int umocktypes_are_equal_int(const int* left, const int* right);
```

**SRS_UMOCKTYPES_C_01_006: [** umocktypes_are_equal_int shall compare the 2 ints pointed to by left and right. **]**
**SRS_UMOCKTYPES_C_01_007: [** If any of the arguments is NULL, umocktypes_are_equal_int shall return -1. **]**
**SRS_UMOCKTYPES_C_01_008: [** If the int value pointed to by left is equal to the int value pointed to by right, umocktypes_are_equal_int shall return 1. **]**
**SRS_UMOCKTYPES_C_01_009: [** If the int values are different, umocktypes_are_equal_int shall return 0. **]**

##umocktypes_copy_int

```c
extern int umocktypes_copy_int(int* destination, const int* source);
```

**SRS_UMOCKTYPES_C_01_010: [** umocktypes_copy_int shall copy the int value from source to destination. **]**
**SRS_UMOCKTYPES_C_01_011: [** On success umocktypes_copy_int shall return 0. **]**
**SRS_UMOCKTYPES_C_01_012: [** If source or destination are NULL, umocktypes_copy_int shall return a non-zero value. **]**

##umocktypes_free_int

```c
extern void umocktypes_free_int(int* value);
```

**SRS_UMOCKTYPES_C_01_013: [** umocktypes_free_int shall do nothing. **]**
