// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <string.h>
#include "umockvalue.h"
#include "umockvalue_charptr.h"

char* umockvalue_stringify_charptr(const char** value)
{
    char* result;

    if (value == NULL)
    {
        result = NULL;
    }
    else
    {
        size_t length = strlen(*value);
        result = (char*)malloc(length + 3);
        if (result != NULL)
        {
            result[0] = '\"';
            (void)memcpy(result + 1, *value, length);
            result[length + 1] = '\"';
            result[length + 2] = '\0';
        }
    }

    return result;
}

int umockvalue_are_equal_charptr(const char** left, const char** right)
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
        result = (strcmp(*left, *right) == 0) ? 1 : 0;
    }

    return result;
}

int umockvalue_copy_charptr(char** destination, const char** source)
{
    int result;
    size_t source_length = strlen(*source);
    *destination = (char*)malloc(source_length + 1);
    if (*destination == NULL)
    {
        result = __LINE__;
    }
    else
    {
        (void)memcpy(*destination, *source, source_length + 1);
        result = 0;
    }

    return 0;
}

void umockvalue_free_charptr(char** value)
{
    if (value != NULL)
    {
        free(*value);
    }
}
