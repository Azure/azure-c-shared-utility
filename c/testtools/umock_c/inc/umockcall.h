// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef UMOCKCALL_H
#define UMOCKCALL_H

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

    typedef struct UMOCKCALL_TAG* UMOCKCALL_HANDLE;
    typedef int(*UMOCKCALL_DATA_CLONE_FUNC)(void* destination, const void* source);
    typedef void(*UMOCKCALL_DATA_FREE_FUNC)(void* umockcall_data);
    typedef char*(*UMOCKCALL_DATA_STRINGIFY_FUNC)(void* umockcall_data);
    typedef int(*UMOCKCALL_DATA_ARE_EQUAL_FUNC)(void* left, void* right);

    extern UMOCKCALL_HANDLE umockcall_create(const char* function_name, void* umockcall_data, void* set_return_func, void* ignore_all_arguments_func, UMOCKCALL_DATA_CLONE_FUNC umockcall_data_clone, UMOCKCALL_DATA_FREE_FUNC umockcall_data_free, UMOCKCALL_DATA_STRINGIFY_FUNC umockcall_data_stringify, UMOCKCALL_DATA_ARE_EQUAL_FUNC umockcall_data_are_equal);
    extern void umockcall_destroy(UMOCKCALL_HANDLE umockcall);
    extern int umockcall_are_equal(UMOCKCALL_HANDLE left, UMOCKCALL_HANDLE right);
    extern char* umockcall_to_string(UMOCKCALL_HANDLE umockcall);
    extern void* umockcall_get_call_data(UMOCKCALL_HANDLE umockcall);

#ifdef __cplusplus
}
#endif

#endif /* UMOCKCALL_H */
