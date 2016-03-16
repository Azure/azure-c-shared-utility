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

##umocktypes_stringify_char

```c
extern char* umocktypes_stringify_char(const char* value);
```

**SRS_UMOCKTYPES_C_01_002: [** umocktypes_stringify_char shall return the string representation of value. **]**
**SRS_UMOCKTYPES_C_01_003: [** If value is NULL, umocktypes_stringify_char shall return NULL. **]**
**SRS_UMOCKTYPES_C_01_004: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_char shall return NULL. **]**
**SRS_UMOCKTYPES_C_01_005: [** If any other error occurs when creating the string representation, umocktypes_stringify_char shall return NULL. **]**

##umocktypes_are_equal_char

```c
extern int umocktypes_are_equal_char(const char* left, const char* right);
```

**SRS_UMOCKTYPES_C_01_006: [** umocktypes_are_equal_char shall compare the 2 chars pointed to by left and right. **]**
**SRS_UMOCKTYPES_C_01_007: [** If any of the arguments is NULL, umocktypes_are_equal_char shall return -1. **]**
**SRS_UMOCKTYPES_C_01_008: [** If the values pointed to by left and right are equal, umocktypes_are_equal_char shall return 1. **]**
**SRS_UMOCKTYPES_C_01_009: [** If the values pointed to by left and right are different, umocktypes_are_equal_char shall return 0. **]**

##umocktypes_copy_char

```c
extern int umocktypes_copy_char(char* destination, const char* source);
```

**SRS_UMOCKTYPES_C_01_010: [** umocktypes_copy_char shall copy the char value from source to destination. **]**
**SRS_UMOCKTYPES_C_01_011: [** On success umocktypes_copy_char shall return 0. **]**
**SRS_UMOCKTYPES_C_01_012: [** If source or destination are NULL, umocktypes_copy_char shall return a non-zero value. **]**

##umocktypes_free_char

```c
extern void umocktypes_free_char(char* value);
```

**SRS_UMOCKTYPES_C_01_013: [** umocktypes_free_char shall do nothing. **]**

##umocktypes_stringify_unsignedchar

```c
extern unsigned char* umocktypes_stringify_unsignedchar(const unsigned char* value);
```

**SRS_UMOCKTYPES_C_01_014: [** umocktypes_stringify_unsignedchar shall return the string representation of value. **]**
**SRS_UMOCKTYPES_C_01_015: [** If value is NULL, umocktypes_stringify_unsignedchar shall return NULL. **]**
**SRS_UMOCKTYPES_C_01_016: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_unsignedchar shall return NULL. **]**
**SRS_UMOCKTYPES_C_01_017: [** If any other error occurs when creating the string representation, umocktypes_stringify_unsignedchar shall return NULL. **]**

##umocktypes_are_equal_unsignedchar

```c
extern int umocktypes_are_equal_unsignedchar(const unsigned char* left, const unsigned char* right);
```

**SRS_UMOCKTYPES_C_01_018: [** umocktypes_are_equal_unsignedchar shall compare the 2 unsigned chars pointed to by left and right. **]**
**SRS_UMOCKTYPES_C_01_019: [** If any of the arguments is NULL, umocktypes_are_equal_unsignedchar shall return -1. **]**
**SRS_UMOCKTYPES_C_01_020: [** If the values pointed to by left and right are equal, umocktypes_are_equal_unsignedchar shall return 1. **]**
**SRS_UMOCKTYPES_C_01_021: [** If the values pointed to by left and right are different, umocktypes_are_equal_unsignedchar shall return 0. **]**

##umocktypes_copy_unsignedchar

```c
extern int umocktypes_copy_unsignedchar(unsigned char* destination, const unsigned char* source);
```

**SRS_UMOCKTYPES_C_01_022: [** umocktypes_copy_unsignedchar shall copy the unsigned char value from source to destination. **]**
**SRS_UMOCKTYPES_C_01_023: [** On success umocktypes_copy_unsignedchar shall return 0. **]**
**SRS_UMOCKTYPES_C_01_024: [** If source or destination are NULL, umocktypes_copy_unsignedchar shall return a non-zero value. **]**

##umocktypes_free_unsignedchar

```c
extern void umocktypes_free_unsignedchar(unsigned char* value);
```

**SRS_UMOCKTYPES_C_01_025: [** umocktypes_free_unsignedchar shall do nothing. **]**

##umocktypes_stringify_short

```c
extern short* umocktypes_stringify_short(const short* value);
```

**SRS_UMOCKTYPES_C_01_026: [** umocktypes_stringify_short shall return the string representation of value. **]**
**SRS_UMOCKTYPES_C_01_027: [** If value is NULL, umocktypes_stringify_short shall return NULL. **]**
**SRS_UMOCKTYPES_C_01_028: [** If allocating a new string to hold the string representation fails, umocktypes_stringify_short shall return NULL. **]**
**SRS_UMOCKTYPES_C_01_029: [** If any other error occurs when creating the string representation, umocktypes_stringify_short shall return NULL. **]**

##umocktypes_are_equal_short

```c
extern int umocktypes_are_equal_short(const short* left, const short* right);
```

**SRS_UMOCKTYPES_C_01_030: [** umocktypes_are_equal_short shall compare the 2 shorts pointed to by left and right. **]**
**SRS_UMOCKTYPES_C_01_031: [** If any of the arguments is NULL, umocktypes_are_equal_short shall return -1. **]**
**SRS_UMOCKTYPES_C_01_032: [** If the values pointed to by left and right are equal, umocktypes_are_equal_short shall return 1. **]**
**SRS_UMOCKTYPES_C_01_033: [** If the values pointed to by left and right are different, umocktypes_are_equal_short shall return 0. **]**

##umocktypes_copy_short

```c
extern int umocktypes_copy_short(short* destination, const short* source);
```

**SRS_UMOCKTYPES_C_01_034: [** umocktypes_copy_short shall copy the short value from source to destination. **]**
**SRS_UMOCKTYPES_C_01_035: [** On success umocktypes_copy_short shall return 0. **]**
**SRS_UMOCKTYPES_C_01_036: [** If source or destination are NULL, umocktypes_copy_short shall return a non-zero value. **]**

##umocktypes_free_short

```c
extern void umocktypes_free_short(short* value);
```

**SRS_UMOCKTYPES_C_01_037: [** umocktypes_free_short shall do nothing. **]**

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
**SRS_UMOCKTYPES_C_01_008: [** If the values pointed to by left and right are equal, umocktypes_are_equal_int shall return 1. **]**
**SRS_UMOCKTYPES_C_01_009: [** If the values pointed to by left and right are different, umocktypes_are_equal_int shall return 0. **]**

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

