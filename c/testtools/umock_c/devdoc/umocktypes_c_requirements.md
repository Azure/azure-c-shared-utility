#umocktypes_c requirements
 
#Overview

umocktypes_c is a module that exposes out of the box functionality for most C supported types. This includes integers, floats, size_t, etc.

#Exposed API

```c
extern int umocktypes_c_register_types(void);

extern char* umocktypes_stringify_int(const int* value);
extern int umocktypes_are_equal_int(const int* left, const int* right);
extern int umocktypes_copy_int(int* destination, const int* source);
extern void umocktypes_free_int(int* value);

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

umocktypes_stringify_int shall return the string representation of value.
If value is NULL, umocktypes_stringify_charptr shall return NULL.
If allocating a new string to hold the string representation fails, umocktypes_stringify_charptr shall return NULL.

##umocktypes_are_equal_charptr

```c
extern int umocktypes_are_equal_int(const int* left, const int* right);
```

umocktypes_are_equal_int shall compare the 2 ints pointed to by left and right.
If any of the arguments is NULL, umocktypes_are_equal_int shall return -1.
If the int value pointed to by left is equal to the int value pointed to by right, umocktypes_are_equal_int shall return 1.
If the int values are different, umocktypes_are_equal_int shall return 0.

##umocktypes_copy_charptr

```c
extern int umocktypes_copy_int(int* destination, const int* source);
```

umocktypes_copy_int shall copy the int value from source to destination.
On success umocktypes_copy_int shall return 0.
If source or destination are NULL, umocktypes_copy_int shall return a non-zero value.

##umocktypes_free_charptr

```c
extern void umocktypes_free_int(int* value);
```

umocktypes_free_int shall do nothing.

