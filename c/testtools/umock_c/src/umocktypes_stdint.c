// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <string.h>
#include "umockvalue.h"
#include "umocktypes_stdint.h"

int umocktypes_stdint_register_types(void)
{
    int result;

    if (umocktypes_register_type("int", (UMOCKVALUE_STRINGIFY_FUNC)umocktypes_stringify_int, (UMOCKVALUE_ARE_EQUAL_FUNC)umocktypes_are_equal_int, (UMOCKVALUE_COPY_FUNC)umocktypes_copy_int, (UMOCKVALUE_FREE_FUNC)umocktypes_free_int) != 0)
    {
        result = __LINE__;
    }
    else
    {
        result = 0;
    }

    return result;
}

char* umocktypes_stringify_int(const int* value)
{
    char* result;

    if (value == NULL)
    {
        result = NULL;
    }
    else
    {
        // adjust this to account for accurate amount of chars
        char temp_buffer[32];
        int length = sprintf(temp_buffer, "%d", *value);
        if (length < 0)
        {
            result = NULL;
        }
        else
        {
            result = (char*)malloc(length + 1);
            if (result != NULL)
            {
                memcpy(result, temp_buffer, length + 1);
            }
        }
    }

    return result;
}

int umocktypes_copy_int(int* destination, const int* source)
{
    int result;

    if ((destination == NULL) ||
        (source == NULL))
    {
        result = __LINE__;
    }
    else
    {
        *destination = *source;
        result = 0;
    }

    return result;
}

void umocktypes_free_int(int* value)
{
    /* no free required for int */
}

int umocktypes_are_equal_int(const int* left, const int* right)
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
        result = ((*left) == (*right)) ? 1 : 0;
    }

    return result;
}
