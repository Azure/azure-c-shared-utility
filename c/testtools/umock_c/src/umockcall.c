// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stddef.h>
#include <string.h>
#include "umockcall.h"

typedef struct MOCK_CALL_TAG
{
    char* function_name;
    void* mock_call_data;
    void* set_return_func;
    void* ignore_all_arguments_func;
    MOCK_CALL_DATA_CLONE_FUNC mock_call_data_clone;
    MOCK_CALL_DATA_FREE_FUNC mock_call_data_free;
    MOCK_CALL_DATA_STRINGIFY_FUNC mock_call_data_stringify;
    MOCK_CALL_DATA_ARE_EQUAL_FUNC mock_call_data_are_equal;
} MOCK_CALL;

MOCK_CALL_HANDLE umockcall_create(const char* function_name, void* mock_call_data, void* set_return_func, void* ignore_all_arguments_func, MOCK_CALL_DATA_CLONE_FUNC mock_call_data_clone, MOCK_CALL_DATA_FREE_FUNC mock_call_data_free, MOCK_CALL_DATA_STRINGIFY_FUNC mock_call_data_stringify, MOCK_CALL_DATA_ARE_EQUAL_FUNC mock_call_data_are_equal)
{
    MOCK_CALL* result = (MOCK_CALL*)malloc(sizeof(MOCK_CALL));
    if (result != NULL)
    {
        size_t function_name_length = strlen(function_name);
        result->function_name = (char*)malloc(function_name_length + 1);
        if (result->function_name == NULL)
        {
            free(result);
            result = NULL;
        }
        else
        {
            (void)memcpy(result->function_name, function_name, function_name_length + 1);
            result->mock_call_data = mock_call_data;
            result->set_return_func = set_return_func;
            result->ignore_all_arguments_func = ignore_all_arguments_func;
            result->mock_call_data_clone = mock_call_data_clone;
            result->mock_call_data_free = mock_call_data_free;
            result->mock_call_data_stringify = mock_call_data_stringify;
            result->mock_call_data_are_equal = mock_call_data_are_equal;
        }
    }

    return result;
}

void umockcall_destroy(MOCK_CALL_HANDLE mock_call)
{
    free(mock_call->mock_call_data);
    free(mock_call);
}

int umockcall_are_equal(MOCK_CALL_HANDLE left, MOCK_CALL_HANDLE right)
{
    int result;

    if (left == right)
    {
        result = 1;
    }
    else if ((left == NULL) || (right == NULL))
    {
        result = 0;
    }
    else
    {
        if (strcmp(left->function_name, right->function_name) != 0)
        {
            result = 0;
        }
        else
        {
            result = left->mock_call_data_are_equal(left->mock_call_data, right->mock_call_data);
        }
    }

    return result;
}

char* umockcall_to_string(MOCK_CALL_HANDLE mock_call)
{
    char* result;

    if (mock_call == NULL)
    {
        result = NULL;
    }
    else
    {
        char* stringified_args = mock_call->mock_call_data_stringify(mock_call->mock_call_data);
        if (stringified_args == NULL)
        {
            result = NULL;
        }
        else
        {
            size_t function_name_length = strlen(mock_call->function_name);
            size_t stringified_args_length = strlen(stringified_args);
        
            /* 4 because () and [] */
            size_t call_length = function_name_length + stringified_args_length + 4;

            result = (char*)malloc(call_length + 1);
            if (result != NULL)
            {
                result[0] = '[';
                (void)memcpy(&result[1], mock_call->function_name, function_name_length);
                result[function_name_length + 1] = '(';
                (void)memcpy(&result[function_name_length + 2], stringified_args, stringified_args_length);
                result[function_name_length + stringified_args_length + 2] = ')';
                result[function_name_length + stringified_args_length + 3] = ']';
                result[function_name_length + stringified_args_length + 4] = '\0';
            }
        }
    }

    return result;
}

void* umockcall_get_call_data(MOCK_CALL_HANDLE mock_call)
{
    void* mock_call_data;

    if (mock_call == NULL)
    {
        mock_call_data = NULL;
    }
    else
    {
        mock_call_data = mock_call->mock_call_data;
    }

    return mock_call_data;
}
