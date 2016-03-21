// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <string.h>

#include "umockcallrecorder.h"
#include "umockcall.h"
#include "umocktypes.h"

typedef struct UMOCKCALLRECORDER_TAG
{
    size_t expected_call_count;
    UMOCKCALL_HANDLE* expected_calls;
    size_t actual_call_count;
    UMOCKCALL_HANDLE* actual_calls;
    char* expected_calls_string;
    char* actual_calls_string;
} UMOCKCALLRECORDER;

UMOCKCALLRECORDER_HANDLE umockcallrecorder_create(void)
{
    UMOCKCALLRECORDER_HANDLE result;

    result = (UMOCKCALLRECORDER_HANDLE)malloc(sizeof(UMOCKCALLRECORDER));
    if (result != NULL)
    {
        result->expected_call_count = 0;
        result->expected_calls = NULL;
        result->expected_calls_string = NULL;
        result->actual_call_count = 0;
        result->actual_calls = NULL;
        result->actual_calls_string = NULL;
    }

    return result;
}

void umockcallrecorder_destroy(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    if (umock_call_recorder != NULL)
    {
        (void)umockcallrecorder_reset_all_calls(umock_call_recorder);

        free(umock_call_recorder->actual_calls_string);
        free(umock_call_recorder->expected_calls_string);
        free(umock_call_recorder);
    }
}

int umockcallrecorder_reset_all_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
	if (umock_call_recorder->expected_calls != NULL)
	{
        size_t i;
        for (i = 0; i < umock_call_recorder->expected_call_count; i++)
        {
            umockcall_destroy(umock_call_recorder->expected_calls[i]);
        }

        free(umock_call_recorder->expected_calls);
        umock_call_recorder->expected_calls = NULL;
	}
    umock_call_recorder->expected_call_count = 0;

    if (umock_call_recorder->actual_calls != NULL)
    {
        size_t i;
        for (i = 0; i < umock_call_recorder->actual_call_count; i++)
        {
            umockcall_destroy(umock_call_recorder->actual_calls[i]);
        }

        free(umock_call_recorder->actual_calls);
        umock_call_recorder->actual_calls = NULL;
    }
    umock_call_recorder->actual_call_count = 0;

	return 0;
}

int umockcallrecorder_add_expected_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, UMOCKCALL_HANDLE mock_call)
{
    int result;
    UMOCKCALL_HANDLE* new_expected_calls = (UMOCKCALL_HANDLE*)realloc(umock_call_recorder->expected_calls, sizeof(UMOCKCALL_HANDLE) * (umock_call_recorder->expected_call_count + 1));
	if (new_expected_calls == NULL)
	{
		result = __LINE__;
	}
	else
	{
        umock_call_recorder->expected_calls = new_expected_calls;
        umock_call_recorder->expected_calls[umock_call_recorder->expected_call_count++] = mock_call;
        result = 0;
	}

    return result;
}

int umockcallrecorder_add_actual_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder, UMOCKCALL_HANDLE mock_call, UMOCKCALL_HANDLE* matched_call)
{
    int result;
    size_t i;
    unsigned int is_error = 0;

    *matched_call = NULL;

    /* Codes_SRS_UMOCK_C_01_115: [ umock_c shall compare calls in order. ]*/
    for (i = 0; i < umock_call_recorder->expected_call_count; i++)
    {
        int are_equal_result = umockcall_are_equal(umock_call_recorder->expected_calls[i], mock_call);
        if (are_equal_result == 1)
        {
            *matched_call = umock_call_recorder->expected_calls[i];
            (void)memmove(&umock_call_recorder->expected_calls[i], &umock_call_recorder->expected_calls[i + 1], (umock_call_recorder->expected_call_count - i - 1) * sizeof(UMOCKCALL_HANDLE));
            umock_call_recorder->expected_call_count--;
            i--;
            break;
        }
        else if (are_equal_result != 0)
        {
            is_error = 1;
            break;
        }
    }

    if (is_error)
    {
        result = __LINE__;
    }
    else
    {
        if (i == umock_call_recorder->expected_call_count)
        {
            /* an unexpected call */
            UMOCKCALL_HANDLE* new_actual_calls = (UMOCKCALL_HANDLE*)realloc(umock_call_recorder->actual_calls, sizeof(UMOCKCALL_HANDLE) * (umock_call_recorder->actual_call_count + 1));
            if (new_actual_calls == NULL)
            {
                result = __LINE__;
            }
            else
            {
                umock_call_recorder->actual_calls = new_actual_calls;
                umock_call_recorder->actual_calls[umock_call_recorder->actual_call_count++] = mock_call;
                result = 0;
            }
        }
        else
        {
            umockcall_destroy(mock_call);
            result = 0;
        }
    }

    return result;
}

const char* umockcallrecorder_get_expected_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    const char* result;
    size_t i;
    char* new_expected_calls_string;

    new_expected_calls_string = (char*)realloc(umock_call_recorder->expected_calls_string, 1);
    if (new_expected_calls_string == NULL)
    {
        result = NULL;
    }
    else
    {
        size_t current_length = 0;
        umock_call_recorder->expected_calls_string = new_expected_calls_string;
        umock_call_recorder->expected_calls_string[0] = '\0';

        for (i = 0; i < umock_call_recorder->expected_call_count; i++)
        {
            char* stringified_call = umockcall_stringify(umock_call_recorder->expected_calls[i]);
            if (stringified_call == NULL)
            {
                break;
            }
            else
            {
                size_t stringified_call_length = strlen(stringified_call);
                new_expected_calls_string = (char*)realloc(umock_call_recorder->expected_calls_string, current_length + stringified_call_length + 1);
                if (new_expected_calls_string == NULL)
                {
                    break;
                }
                else
                {
                    umock_call_recorder->expected_calls_string = new_expected_calls_string;
                    (void)memcpy(umock_call_recorder->expected_calls_string + current_length, stringified_call, stringified_call_length + 1);
                    current_length += stringified_call_length;
                }

                free(stringified_call);
            }
        }

        if (i < umock_call_recorder->expected_call_count)
        {
            result = NULL;
        }
        else
        {
            result = umock_call_recorder->expected_calls_string;
        }
    }

    return result;
}

const char* umockcallrecorder_get_actual_calls(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    const char* result;
    size_t i;
    char* new_actual_calls_string;

    new_actual_calls_string = (char*)realloc(umock_call_recorder->actual_calls_string, 1);
    if (new_actual_calls_string == NULL)
    {
        result = NULL;
    }
    else
    {
        size_t current_length = 0;
        umock_call_recorder->actual_calls_string = new_actual_calls_string;
        umock_call_recorder->actual_calls_string[0] = '\0';

        for (i = 0; i < umock_call_recorder->actual_call_count; i++)
        {
            char* stringified_call = umockcall_stringify(umock_call_recorder->actual_calls[i]);
            if (stringified_call == NULL)
            {
                break;
            }
            else
            {
                size_t stringified_call_length = strlen(stringified_call);
                new_actual_calls_string = (char*)realloc(umock_call_recorder->actual_calls_string, current_length + stringified_call_length + 1);
                if (new_actual_calls_string == NULL)
                {
                    break;
                }
                else
                {
                    umock_call_recorder->actual_calls_string = new_actual_calls_string;
                    (void)memcpy(umock_call_recorder->actual_calls_string + current_length, stringified_call, stringified_call_length + 1);
                    current_length += stringified_call_length;
                }

                free(stringified_call);
            }
        }

        if (i < umock_call_recorder->actual_call_count)
        {
            result = NULL;
        }
        else
        {
            result = umock_call_recorder->actual_calls_string;
        }
    }

    return result;
}

UMOCKCALL_HANDLE umockcallrecorder_get_last_expected_call(UMOCKCALLRECORDER_HANDLE umock_call_recorder)
{
    UMOCKCALL_HANDLE result;

    if (umock_call_recorder->expected_call_count == 0)
    {
        result = NULL;
    }
    else
    {
        result = umock_call_recorder->expected_calls[umock_call_recorder->expected_call_count - 1];
    }

    return result;
}
