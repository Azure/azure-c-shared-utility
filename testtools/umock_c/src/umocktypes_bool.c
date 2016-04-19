// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "azure_c_shared_utility/macro_utils.h"
#include "umocktypes.h"
#include "umock_c.h"
#include "umocktypes_bool.h"

char* umocktypes_stringify_bool(const bool* value)
{
    char* result;
    if (value == NULL)
    {
        result = NULL;
    }
    else
    {
        const char* stringified_bool = *value ? "true" : "false";
        int length = strlen(stringified_bool);
        if (length < 0)
        {
            result = NULL;
        }
        else
        {
            result = (char*)malloc(length + 1);
            if (result != NULL)
            {
                (void)memcpy(result, stringified_bool, length + 1);
            }
        }
    }
    return result;
}

int umocktypes_are_equal_bool(const bool* left, const bool* right)
{
    int result;
    if ((left == NULL) || (right == NULL))
    {
        result = -1;
    }
    else
    {
        result = ((*left) == (*right)) ? 1 : 0;
    }
    return result;
}

int umocktypes_copy_bool(bool* destination, const bool* source)
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

void umocktypes_free_bool(bool* value)
{
}

int umocktypes_bool_register_types(void)
{
    int result;

    if ((umocktypes_register_type("bool", umocktypes_stringify_bool, umocktypes_are_equal_bool, umocktypes_copy_bool, umocktypes_free_bool) != 0) ||
        (umocktypes_register_alias_type("_Bool", "bool") != 0))
    {
        result = __LINE__;
    }
    else
    {
        result = 0;
    }

    return result;
}
