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
        /* Codes_SRS_UMOCKVALUE_CHARPTR_01_004: [ If value is NULL, umockvalue_stringify_charptr shall return NULL. ]*/
        result = NULL;
    }
    else
    {
        /* Codes_SRS_UMOCKVALUE_CHARPTR_01_002: [ umockvalue_stringify_charptr shall return a string containing the string representation of value, enclosed by quotes (\"value\"). ] */
        size_t length = strlen(*value);
        result = (char*)malloc(length + 3);
        /* Codes_SRS_UMOCKVALUE_CHARPTR_01_003: [ If allocating a new string to hold the string representation fails, umockvalue_stringify_charptr shall return NULL. ]*/
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

/* Codes_SRS_UMOCKVALUE_CHARPTR_01_005: [ umockvalue_are_equal_charptr shall compare the 2 strings pointed to by left and right. ] */
int umockvalue_are_equal_charptr(const char** left, const char** right)
{
    int result;

    /* Codes_SRS_UMOCKVALUE_CHARPTR_01_007: [ If left or right are equal, umockvalue_are_equal_charptr shall return 1. ]*/
    if (left == right)
    {
        result = 1;
    }
    else if ((left == NULL) || (right == NULL))
    {
        /* Codes_SRS_UMOCKVALUE_CHARPTR_01_008: [ If only one of the left and right argument is NULL, umockvalue_are_equal_charptr shall return 0. ] */
        result = 0;
    }
    else
    {
        /* Codes_SRS_UMOCKVALUE_CHARPTR_01_009: [ If the string pointed to by left is equal to the string pointed to by right, umockvalue_are_equal_charptr shall return 1. ]*/
        /* Codes_SRS_UMOCKVALUE_CHARPTR_01_010: [ If the string pointed to by left is different than the string pointed to by right, umockvalue_are_equal_charptr shall return 0. ]*/
        /* Codes_SRS_UMOCKVALUE_CHARPTR_01_006: [ The comparison shall be case sensitive. ]*/
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
