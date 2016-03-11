// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>
#include <string.h>
#include "umocktypes_stdint.h"
#include "umocktypes.h"

typedef struct UMOCK_VALUE_TYPE_HANDLERS_TAG
{
    char* type;
    UMOCKVALUE_STRINGIFY_FUNC stringify;
    UMOCKVALUE_COPY_FUNC value_copy;
    UMOCKVALUE_FREE_FUNC value_free;
    UMOCKVALUE_ARE_EQUAL_FUNC are_equal;
} UMOCK_VALUE_TYPE_HANDLERS;

static UMOCK_VALUE_TYPE_HANDLERS* type_handlers = NULL;
static size_t type_handler_count = 0;

static UMOCK_VALUE_TYPE_HANDLERS* get_value_type_handlers(const char* type)
{
    UMOCK_VALUE_TYPE_HANDLERS* result;
    size_t i;

    for (i = 0; i < type_handler_count; i++)
    {
        if (strcmp(type_handlers[i].type, type) == 0)
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

int umocktypes_register_type(const char* type, UMOCKVALUE_STRINGIFY_FUNC stringify, UMOCKVALUE_ARE_EQUAL_FUNC are_equal, UMOCKVALUE_COPY_FUNC value_copy, UMOCKVALUE_FREE_FUNC value_free)
{
    int result;

    UMOCK_VALUE_TYPE_HANDLERS* new_type_handlers = (UMOCK_VALUE_TYPE_HANDLERS*)realloc(type_handlers, sizeof(UMOCK_VALUE_TYPE_HANDLERS) * (type_handler_count + 1));
    if (new_type_handlers == NULL)
    {
        result = __LINE__;
    }
    else
    {
        size_t type_string_length = strlen(type);
        type_handlers = new_type_handlers;
        type_handlers[type_handler_count].type = (char*)malloc(type_string_length + 1);
        if (type_handlers[type_handler_count].type == NULL)
        {
            result = __LINE__;
        }
        else
        {
            (void)memcpy(type_handlers[type_handler_count].type, type, type_string_length + 1);
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
