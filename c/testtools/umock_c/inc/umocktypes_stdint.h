// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UMOCKTYPES_CHARPTR_H
#define UMOCKTYPES_CHARPTR_H

#ifdef __cplusplus
extern "C" {
#endif

    extern int umocktypes_stdint_register_types(void);

    extern char* umocktypes_stringify_int(const int* value);
    extern int umocktypes_are_equal_int(const int* left, const int* right);
    extern int umocktypes_copy_int(int* destination, const int* source);
    extern void umocktypes_free_int(int* value);

#ifdef __cplusplus
}
#endif

#endif /* UMOCKTYPES_CHARPTR_H */
