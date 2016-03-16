// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <string.h>
#include "umocktypes.h"
#include "umocktypes_c.h"

int umocktypes_c_register_types(void)
{
    int result;

    if (umocktypes_register_type("int", (UMOCKTYPE_STRINGIFY_FUNC)umocktypes_stringify_int, (UMOCKTYPE_ARE_EQUAL_FUNC)umocktypes_are_equal_int, (UMOCKTYPE_COPY_FUNC)umocktypes_copy_int, (UMOCKTYPE_FREE_FUNC)umocktypes_free_int) != 0)
    {
        result = __LINE__;
    }
    else
    {
        result = 0;
    }

    return result;
}

/* Codes_SRS_UMOCKTYPES_C_01_002: [ umocktypes_stringify_int shall return the string representation of value. ]*/
char* umocktypes_stringify_int(const int* value)
{
    char* result;

    if (value == NULL)
    {
        /* Codes_SRS_UMOCKTYPES_C_01_003: [ If value is NULL, umocktypes_stringify_int shall return NULL. ]*/
        result = NULL;
    }
    else
    {
        // adjust this to account for accurate amount of chars
        char temp_buffer[12];
        int length = sprintf(temp_buffer, "%d", *value);
        if (length < 0)
        {
            /* Codes_SRS_UMOCKTYPES_C_01_005: [ If any other error occurs when creating the string representation, umocktypes_stringify_int shall return NULL. ]*/
            result = NULL;
        }
        else
        {
            result = (char*)malloc(length + 1);
            /* Codes_SRS_UMOCKTYPES_C_01_004: [ If allocating a new string to hold the string representation fails, umocktypes_stringify_int shall return NULL. ]*/
            if (result != NULL)
            {
                (void)memcpy(result, temp_buffer, length + 1);
            }
        }
    }

    return result;
}

/* Codes_SRS_UMOCKTYPES_C_01_006: [ umocktypes_are_equal_int shall compare the 2 ints pointed to by left and right. ]*/
int umocktypes_are_equal_int(const int* left, const int* right)
{
    int result;

    if ((left == NULL) || (right == NULL))
    {
        /* Codes_SRS_UMOCKTYPES_C_01_007: [ If any of the arguments is NULL, umocktypes_are_equal_int shall return -1. ]*/
        result = -1;
    }
    else
    {
        /* Codes_SRS_UMOCKTYPES_C_01_008: [ If the int value pointed to by left is equal to the int value pointed to by right, umocktypes_are_equal_int shall return 1. ]*/
        /* Codes_SRS_UMOCKTYPES_C_01_009: [ If the int values are different, umocktypes_are_equal_int shall return 0. ]*/
        result = ((*left) == (*right)) ? 1 : 0;
    }

    return result;
}

/* Codes_SRS_UMOCKTYPES_C_01_010: [ umocktypes_copy_int shall copy the int value from source to destination. ]*/
int umocktypes_copy_int(int* destination, const int* source)
{
    int result;

    /* Codes_SRS_UMOCKTYPES_C_01_012: [ If source or destination are NULL, umocktypes_copy_int shall return a non-zero value. ]*/
    if ((destination == NULL) ||
        (source == NULL))
    {
        result = __LINE__;
    }
    else
    {
        *destination = *source;

        /* Codes_SRS_UMOCKTYPES_C_01_011: [ On success umocktypes_copy_int shall return 0. ]*/
        result = 0;
    }

    return result;
}

/* Codes_SRS_UMOCKTYPES_C_01_013: [ umocktypes_free_int shall do nothing. ]*/
void umocktypes_free_int(int* value)
{
    /* no free required for int */
}

