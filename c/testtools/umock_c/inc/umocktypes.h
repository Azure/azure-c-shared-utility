// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UMOCKTYPES_H
#define UMOCKTYPES_H

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

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

#ifdef __cplusplus
}
#endif

#endif /* UMOCKTYPES_H */
