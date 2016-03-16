// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UMOCKTYPES_C_H
#define UMOCKTYPES_C_H

#ifdef __cplusplus
extern "C" {
#endif

    extern int umocktypes_c_register_types(void);

    extern char* umocktypes_stringify_char(const char* value);
    extern int umocktypes_are_equal_char(const char* left, const char* right);
    extern int umocktypes_copy_char(char* destination, const char* source);
    extern void umocktypes_free_char(char* value);

    extern char* umocktypes_stringify_short(const short* value);
    extern int umocktypes_are_equal_short(const short* left, const short* right);
    extern int umocktypes_copy_short(short* destination, const short* source);
    extern void umocktypes_free_short(short* value);

    extern char* umocktypes_stringify_int(const int* value);
    extern int umocktypes_are_equal_int(const int* left, const int* right);
    extern int umocktypes_copy_int(int* destination, const int* source);
    extern void umocktypes_free_int(int* value);

    extern char* umocktypes_stringify_long(const long* value);
    extern int umocktypes_are_equal_long(const long* left, const long* right);
    extern int umocktypes_copy_long(long* destination, const long* source);
    extern void umocktypes_free_long(long* value);

    extern char* umocktypes_stringify_longlong(const long long* value);
    extern int umocktypes_are_equal_longlong(const long long* left, const long long* right);
    extern int umocktypes_copy_longlong(long long* destination, const long long* source);
    extern void umocktypes_free_longlong(long long* value);

    extern char* umocktypes_stringify_unsignedchar(const unsigned char* value);
    extern int umocktypes_are_equal_unsignedchar(const unsigned char* left, const unsigned char* right);
    extern int umocktypes_copy_unsignedchar(unsigned char* destination, const unsigned char* source);
    extern void umocktypes_free_unsignedchar(unsigned char* value);

    extern char* umocktypes_stringify_unsignedshort(const unsigned short* value);
    extern int umocktypes_are_equal_unsignedshort(const unsigned short* left, const unsigned short* right);
    extern int umocktypes_copy_unsignedshort(unsigned short* destination, const unsigned short* source);
    extern void umocktypes_free_unsignedshort(unsigned short* value);

    extern char* umocktypes_stringify_unsignedint(const unsigned int* value);
    extern int umocktypes_are_equal_unsignedint(const unsigned int* left, const unsigned int* right);
    extern int umocktypes_copy_unsignedint(unsigned int* destination, const unsigned int* source);
    extern void umocktypes_free_unsignedint(unsigned int* value);

    extern char* umocktypes_stringify_unsignedlong(const unsigned long* value);
    extern int umocktypes_are_equal_unsignedlong(const unsigned long* left, const unsigned long* right);
    extern int umocktypes_copy_unsignedlong(unsigned long* destination, const unsigned long* source);
    extern void umocktypes_free_unsignedlong(unsigned long* value);

    extern char* umocktypes_stringify_unsignedlonglong(const unsigned long long* value);
    extern int umocktypes_are_equal_unsignedlonglong(const unsigned long long* left, const unsigned long long* right);
    extern int umocktypes_copy_unsignedlonglong(unsigned long long* destination, const unsigned long long* source);
    extern void umocktypes_free_unsignedlonglong(unsigned long long* value);

#ifdef __cplusplus
}
#endif

#endif /* UMOCKTYPES_C_H */
