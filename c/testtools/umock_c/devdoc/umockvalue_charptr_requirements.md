#umockvalue_charptrl requirements
 
#Overview

umockvalue_charptr is a module exposes out of the box functionality for char* and const char* types for umockc.

#Exposed API

```c
extern int umockvalue_charptr_register_types(void);

extern char* umockvalue_stringify_charptr(const char** value);
extern int umockvalue_are_equal_charptr(const char** left, const char** right);
extern int umockvalue_copy_charptr(char** destination, const char** source);
extern void umockvalue_free_charptr(char** value);

extern char* umockvalue_stringify_const_charptr(const char** value);
extern int umockvalue_are_equal_const_charptr(const char** left, const char** right);
extern int umockvalue_copy_const_charptr(char** destination, const char** source);
extern void umockvalue_free_const_charptr(char** value);
```

##umockvalue_charptr_register_types

```c
extern int umockvalue_charptr_register_types(void);
```

**SRS_UMOCKVALUE_CHARPTR_01_001: [** umockvalue_charptr_register_types shall register support for the types char\* and const char\* by using the REGISTER_UMOCK_VALUE_TYPE macro provided by umockc. **]**

##umockvalue_stringify_charptr

```c
extern char* umockvalue_stringify_charptr(const char** value);
```

XX**SRS_UMOCKVALUE_CHARPTR_01_002: [** umockvalue_stringify_charptr shall return a string containing the string representation of value, enclosed by quotes (\"value\"). **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_004: [** If value is NULL, umockvalue_stringify_charptr shall return NULL. **]**
**SRS_UMOCKVALUE_CHARPTR_01_003: [** If allocating a new string to hold the string representation fails, umockvalue_stringify_charptr shall return NULL. **]**

##umockvalue_are_equal_charptr

```c
extern int umockvalue_are_equal_charptr(const char** left, const char** right);
```

XX**SRS_UMOCKVALUE_CHARPTR_01_005: [** umockvalue_are_equal_charptr shall compare the 2 strings pointed to by left and right. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_006: [** The comparison shall be case sensitive. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_007: [** If left and right are equal, umockvalue_are_equal_charptr shall return 1. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_008: [** If only one of the left and right argument is NULL, umockvalue_are_equal_charptr shall return 0. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_009: [** If the string pointed to by left is equal to the string pointed to by right, umockvalue_are_equal_charptr shall return 1. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_010: [** If the string pointed to by left is different than the string pointed to by right, umockvalue_are_equal_charptr shall return 0. **]**

##umockvalue_copy_charptr

```c
extern int umockvalue_copy_charptr(char** destination, const char** source);
```

XX**SRS_UMOCKVALUE_CHARPTR_01_011: [** umockvalue_copy_charptr shall allocate a new sequence of chars by using malloc. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_012: [** The number of bytes allocated shall accomodate the string pointed to by source. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_014: [** umockvalue_copy_charptr shall copy the string pointed to by source to the newly allocated memory. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_015: [** The newly allocated string shall be returned in the destination argument. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_016: [** On success umockvalue_copy_charptr shall return 0. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_013: [** If source or destination are NULL, umockvalue_copy_charptr shall return a non-zero value. **]**
**SRS_UMOCKVALUE_CHARPTR_01_036: [** If allocating the memory for the new string fails, umockvalue_copy_charptr shall fail and return a non-zero value. **]**

##umockvalue_free_charptr

```c
extern void umockvalue_free_charptr(char** value);
```

**SRS_UMOCKVALUE_CHARPTR_01_017: [** umockvalue_free_charptr shall free the string pointed to by value. **]**
**SRS_UMOCKVALUE_CHARPTR_01_018: [** If value is NULL, umockvalue_free_charptr shall do nothing. **]**

##umockvalue_stringify_const_charptr

```c
extern char* umockvalue_stringify_const_charptr(const char** value);
```

XX**SRS_UMOCKVALUE_CHARPTR_01_019: [** umockvalue_stringify_const_charptr shall return a string containing the string representation of value, enclosed by quotes (\"value\"). **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_020: [** If value is NULL, umockvalue_stringify_const_charptr shall return NULL. **]**
**SRS_UMOCKVALUE_CHARPTR_01_021: [** If allocating a new string to hold the string representation fails, umockvalue_stringify_const_charptr shall return NULL. **]**

##umockvalue_are_equal_const_charptr

```c
extern int umockvalue_are_equal_const_charptr(const char** left, const char** right);
```

XX**SRS_UMOCKVALUE_CHARPTR_01_022: [** umockvalue_are_equal_const_charptr shall compare the 2 strings pointed to by left and right. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_023: [** The comparison shall be case sensitive. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_024: [** If left and right are equal, umockvalue_are_equal_const_charptr shall return 1. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_025: [** If only one of the left and right argument is NULL, umockvalue_are_equal_const_charptr shall return 0. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_026: [** If the string pointed to by left is equal to the string pointed to by right, umockvalue_are_equal_const_charptr shall return 1. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_027: [** If the string pointed to by left is different than the string pointed to by right, umockvalue_are_equal_const_charptr shall return 0. **]**

##umockvalue_copy_const_charptr

```c
extern int umockvalue_copy_const_charptr(char** destination, const char** source);
```

XX**SRS_UMOCKVALUE_CHARPTR_01_028: [** umockvalue_copy_const_charptr shall allocate a new sequence of chars by using malloc. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_029: [** The number of bytes allocated shall accomodate the string pointed to by source. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_030: [** umockvalue_copy_const_charptr shall copy the string pointed to by source to the newly allocated memory. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_031: [** The newly allocated string shall be returned in the destination argument. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_032: [** On success umockvalue_copy_const_charptr shall return 0. **]**
XX**SRS_UMOCKVALUE_CHARPTR_01_033: [** If source or destination are NULL, umockvalue_copy_const_charptr shall return a non-zero value. **]**
**SRS_UMOCKVALUE_CHARPTR_01_037: [** If allocating the memory for the new string fails, umockvalue_copy_const_charptr shall fail and return a non-zero value. **]**

##umockvalue_free_const_charptr

```c
extern void umockvalue_free_const_charptr(char** value);
```

**SRS_UMOCKVALUE_CHARPTR_01_034: [** umockvalue_free_const_charptr shall free the string pointed to by value. **]**
**SRS_UMOCKVALUE_CHARPTR_01_035: [** If value is NULL, umockvalue_free_const_charptr shall do nothing. **]**
