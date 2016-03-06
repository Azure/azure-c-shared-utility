// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stddef.h>
#include <string.h>
#include "umockcall.h"

typedef struct UMOCKCALL_TAG
{
    char* function_name;
    void* umockcall_data;
    UMOCKCALL_DATA_FREE_FUNC umockcall_data_free;
    UMOCKCALL_DATA_STRINGIFY_FUNC umockcall_data_stringify;
    UMOCKCALL_DATA_ARE_EQUAL_FUNC umockcall_data_are_equal;
} MOCK_CALL;

UMOCKCALL_HANDLE umockcall_create(const char* function_name, void* umockcall_data, UMOCKCALL_DATA_FREE_FUNC umockcall_data_free, UMOCKCALL_DATA_STRINGIFY_FUNC umockcall_data_stringify, UMOCKCALL_DATA_ARE_EQUAL_FUNC umockcall_data_are_equal)
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
            result->umockcall_data = umockcall_data;
            result->umockcall_data_free = umockcall_data_free;
            result->umockcall_data_stringify = umockcall_data_stringify;
            result->umockcall_data_are_equal = umockcall_data_are_equal;
        }
    }

    return result;
}

void umockcall_destroy(UMOCKCALL_HANDLE umockcall)
{
    free(umockcall->umockcall_data);
    free(umockcall);
}

int umockcall_are_equal(UMOCKCALL_HANDLE left, UMOCKCALL_HANDLE right)
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
            result = left->umockcall_data_are_equal(left->umockcall_data, right->umockcall_data);
        }
    }

    return result;
}

char* umockcall_to_string(UMOCKCALL_HANDLE umockcall)
{
    char* result;

    if (umockcall == NULL)
    {
        result = NULL;
    }
    else
    {
        char* stringified_args = umockcall->umockcall_data_stringify(umockcall->umockcall_data);
        if (stringified_args == NULL)
        {
            result = NULL;
        }
        else
        {
            size_t function_name_length = strlen(umockcall->function_name);
            size_t stringified_args_length = strlen(stringified_args);
        
            /* 4 because () and [] */
            size_t call_length = function_name_length + stringified_args_length + 4;

            result = (char*)malloc(call_length + 1);
            if (result != NULL)
            {
                result[0] = '[';
                (void)memcpy(&result[1], umockcall->function_name, function_name_length);
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

void* umockcall_get_call_data(UMOCKCALL_HANDLE umockcall)
{
    void* umockcall_data;

    if (umockcall == NULL)
    {
        umockcall_data = NULL;
    }
    else
    {
        umockcall_data = umockcall->umockcall_data;
    }

    return umockcall_data;
}
