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
    typedef void(*UMOCKCALL_DATA_FREE_FUNC)(void* mock_call_data);
    typedef char*(*UMOCKCALL_DATA_STRINGIFY_FUNC)(void* mock_call_data);
    typedef int(*UMOCKCALL_DATA_ARE_EQUAL_FUNC)(void* left, void* right);

    extern UMOCKCALL_HANDLE umockcall_create(const char* function_name, void* mock_call_data, void* set_return_func, void* ignore_all_arguments_func, UMOCKCALL_DATA_CLONE_FUNC mock_call_data_clone, UMOCKCALL_DATA_FREE_FUNC mock_call_data_free, UMOCKCALL_DATA_STRINGIFY_FUNC mock_call_data_stringify, UMOCKCALL_DATA_ARE_EQUAL_FUNC mock_call_data_are_equal);
    extern void umockcall_destroy(UMOCKCALL_HANDLE mock_call);
    extern int umockcall_are_equal(UMOCKCALL_HANDLE left, UMOCKCALL_HANDLE right);
    extern int umockcall_stringified_length(UMOCKCALL_HANDLE mock_call, size_t* length);
    extern char* umockcall_to_string(UMOCKCALL_HANDLE mock_call);
    extern void* umockcall_get_call_data(UMOCKCALL_HANDLE mock_call);

#ifdef __cplusplus
}
#endif

#endif /* UMOCKCALL_H */
