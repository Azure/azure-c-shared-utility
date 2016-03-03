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

    typedef struct MOCK_CALL_TAG* MOCK_CALL_HANDLE;
    typedef int(*MOCK_CALL_DATA_CLONE_FUNC)(void* destination, const void* source);
    typedef void(*MOCK_CALL_DATA_FREE_FUNC)(void* mock_call_data);
    typedef char*(*MOCK_CALL_DATA_STRINGIFY_FUNC)(void* mock_call_data);
    typedef int(*MOCK_CALL_DATA_ARE_EQUAL_FUNC)(void* left, void* right);

    extern MOCK_CALL_HANDLE umockcall_create(const char* function_name, void* mock_call_data, void* set_return_func, void* ignore_all_arguments_func, MOCK_CALL_DATA_CLONE_FUNC mock_call_data_clone, MOCK_CALL_DATA_FREE_FUNC mock_call_data_free, MOCK_CALL_DATA_STRINGIFY_FUNC mock_call_data_stringify, MOCK_CALL_DATA_ARE_EQUAL_FUNC mock_call_data_are_equal);
    extern void umockcall_destroy(MOCK_CALL_HANDLE mock_call);
    extern int umockcall_are_equal(MOCK_CALL_HANDLE left, MOCK_CALL_HANDLE right);
    extern int umockcall_stringified_length(MOCK_CALL_HANDLE mock_call, size_t* length);
    extern char* umockcall_to_string(MOCK_CALL_HANDLE mock_call);
    extern void* umockcall_get_call_data(MOCK_CALL_HANDLE mock_call);

#ifdef __cplusplus
}
#endif

#endif /* UMOCKCALL_H */
