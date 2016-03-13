// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "umocktypes_stdint.h"
#include "umocktypes.h"

typedef struct UMOCK_VALUE_TYPE_HANDLERS_TAG
{
    char* type;
    UMOCKTYPE_STRINGIFY_FUNC stringify;
    UMOCKTYPE_COPY_FUNC value_copy;
    UMOCKTYPE_FREE_FUNC value_free;
    UMOCKTYPE_ARE_EQUAL_FUNC are_equal;
} UMOCK_VALUE_TYPE_HANDLERS;

static UMOCK_VALUE_TYPE_HANDLERS* type_handlers = NULL;
static size_t type_handler_count = 0;

static char* normalize_type(const char* type)
{
    size_t length = 0;
    size_t pos = 0;
    char* result;

    while (type[pos] != '\0')
    {
        if (!((pos > 0) && isspace(type[pos]) && isspace(type[pos - 1])))
        {
            length++;
        }

        pos++;
    }

    result = (char*)malloc(length + 1);
    if (result != NULL)
    {
        pos = 0;
        length = 0;

        while (type[pos] != '\0')
        {
            if (!((pos > 0) && isspace(type[pos]) && isspace(type[pos - 1])))
            {
                result[length] = type[pos];
                length++;
            }

            pos++;
        }

        result[length] = '\0';
    }

    return result;
}

static UMOCK_VALUE_TYPE_HANDLERS* get_value_type_handlers(const char* type)
{
    UMOCK_VALUE_TYPE_HANDLERS* result;

    char* normalized_type = normalize_type(type);
    if (normalized_type == NULL)
    {
        result = NULL;
    }
    else
    {
        size_t i;

        for (i = 0; i < type_handler_count; i++)
        {
            if (strcmp(type_handlers[i].type, normalized_type) == 0)
            {
                break;
            }
        }

        if (i < type_handler_count)
        {
            result = &type_handlers[i];
        }
        else
        {
            result = NULL;
        }

        free(normalized_type);
    }

    return result;
}

int umocktypes_init(void)
{
    return 0;
}

void umocktypes_deinit(void)
{
    size_t i;

    for (i = 0; i < type_handler_count; i++)
    {
        free(type_handlers[i].type);
    }

    free(type_handlers);
    type_handlers = NULL;
}

int umocktypes_register_type(const char* type, UMOCKTYPE_STRINGIFY_FUNC stringify, UMOCKTYPE_ARE_EQUAL_FUNC are_equal, UMOCKTYPE_COPY_FUNC value_copy, UMOCKTYPE_FREE_FUNC value_free)
{
    int result;

    UMOCK_VALUE_TYPE_HANDLERS* new_type_handlers = (UMOCK_VALUE_TYPE_HANDLERS*)realloc(type_handlers, sizeof(UMOCK_VALUE_TYPE_HANDLERS) * (type_handler_count + 1));
    if (new_type_handlers == NULL)
    {
        result = __LINE__;
    }
    else
    {
        type_handlers = new_type_handlers;
        type_handlers[type_handler_count].type = normalize_type(type);
        if (type_handlers[type_handler_count].type == NULL)
        {
            result = __LINE__;
        }
        else
        {
            type_handlers[type_handler_count].stringify = stringify;
            type_handlers[type_handler_count].value_copy = value_copy;
            type_handlers[type_handler_count].value_free = value_free;
            type_handlers[type_handler_count].are_equal = are_equal;
            type_handler_count++;

            result = 0;
        }
    }

    return result;
}

char* umocktypes_stringify(const char* type, const void* value)
{
    char* result;
    UMOCK_VALUE_TYPE_HANDLERS* value_type_handlers = get_value_type_handlers(type);

    if (value_type_handlers == NULL)
    {
        result = NULL;
    }
    else
    {
        result = value_type_handlers->stringify(value);
    }

    return result;
}

int umocktypes_are_equal(const char* type, const void* left, const void* right)
{
    int result;
    UMOCK_VALUE_TYPE_HANDLERS* value_type_handlers = get_value_type_handlers(type);

    if (value_type_handlers == NULL)
    {
        result = 0;
    }
    else
    {
        result = value_type_handlers->are_equal(left, right);
    }

    return result;
}

int umocktypes_copy(const char* type, void* destination, const void* source)
{
    int result;
    UMOCK_VALUE_TYPE_HANDLERS* value_type_handlers = get_value_type_handlers(type);
    if (value_type_handlers == NULL)
    {
        result = __LINE__;
    }
    else
    {
        result = value_type_handlers->value_copy(destination, source);
    }
    return result;
}

void umocktypes_free(const char* type, void* value)
{
    UMOCK_VALUE_TYPE_HANDLERS* value_type_handlers = get_value_type_handlers(type);
    if (value_type_handlers != NULL)
    {
        value_type_handlers->value_free(value);
    }
}
