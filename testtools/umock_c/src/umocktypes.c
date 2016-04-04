// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "umocktypes.h"
#include "umocktypename.h"

typedef struct UMOCK_VALUE_TYPE_HANDLERS_TAG
{
    char* type;
    UMOCKTYPE_STRINGIFY_FUNC stringify_func;
    UMOCKTYPE_COPY_FUNC copy_func;
    UMOCKTYPE_FREE_FUNC free_func;
    UMOCKTYPE_ARE_EQUAL_FUNC are_equal_func;
} UMOCK_VALUE_TYPE_HANDLERS;

typedef enum UMOCKTYPES_STATE_TAG
{
    UMOCKTYPES_STATE_NOT_INITIALIZED,
    UMOCKTYPES_STATE_INITIALIZED
} UMOCKTYPES_STATE;

static UMOCK_VALUE_TYPE_HANDLERS* type_handlers = NULL;
static size_t type_handler_count = 0;
static UMOCKTYPES_STATE umocktypes_state = UMOCKTYPES_STATE_NOT_INITIALIZED;

static UMOCK_VALUE_TYPE_HANDLERS* get_value_type_handlers(const char* type_name)
{
    UMOCK_VALUE_TYPE_HANDLERS* result;

    size_t i;

    for (i = 0; i < type_handler_count; i++)
    {
        if (strcmp(type_handlers[i].type, type_name) == 0)
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

/* Codes_SRS_UMOCKTYPES_01_001: [ umocktypes_init shall initialize the umocktypes module. ] */
int umocktypes_init(void)
{
    int result;

    if (umocktypes_state == UMOCKTYPES_STATE_INITIALIZED)
    {
        /* Codes_SRS_UMOCKTYPES_01_004: [ umocktypes_init after another umocktypes_init without deinitializing the module shall fail and return a non-zero value. ]*/
        result = __LINE__;
    }
    else
    {
        /* Codes_SRS_UMOCKTYPES_01_002: [ After initialization the list of registered type shall be empty. ] */
        type_handlers = NULL;
        type_handler_count = 0;

        umocktypes_state = UMOCKTYPES_STATE_INITIALIZED;

        /* Codes_SRS_UMOCKTYPES_01_003: [ On success umocktypes_init shall return 0. ]*/
        result = 0;
    }

    return result;
}

void umocktypes_deinit(void)
{
    /* Codes_SRS_UMOCKTYPES_01_006: [ If the module was not initialized, umocktypes_deinit shall do nothing. ]*/
    if (umocktypes_state == UMOCKTYPES_STATE_INITIALIZED)
    {
        size_t i;

        /* Codes_SRS_UMOCKTYPES_01_005: [ umocktypes_deinit shall free all resources associated with the registered types and shall leave the module in a state where another init is possible. ]*/
        for (i = 0; i < type_handler_count; i++)
        {
            free(type_handlers[i].type);
        }

        free(type_handlers);
        type_handlers = NULL;
        type_handler_count = 0;

        /* Codes_SRS_UMOCKTYPES_01_040: [ An umocktypes_init call after deinit shall succeed provided all underlying calls succeed. ]*/
        umocktypes_state = UMOCKTYPES_STATE_NOT_INITIALIZED;
    }
}

int umocktypes_register_type(const char* type, UMOCKTYPE_STRINGIFY_FUNC stringify_func, UMOCKTYPE_ARE_EQUAL_FUNC are_equal_func, UMOCKTYPE_COPY_FUNC copy_func, UMOCKTYPE_FREE_FUNC free_func)
{
    int result;

    if ((type == NULL) ||
        (stringify_func == NULL) ||
        (are_equal_func == NULL) ||
        (copy_func == NULL) ||
        (free_func == NULL))
    {
        /* Codes_SRS_UMOCKTYPES_01_009: [ If any of the arguments is NULL, umocktypes_register_type shall fail and return a non-zero value. ]*/
        result = __LINE__;
    }
    /* Codes_SRS_UMOCKTYPES_01_050: [ If umocktypes_register_type is called when the module is not initialized, umocktypes_register_type shall fail and return a non zero value. ]*/
    else if (umocktypes_state != UMOCKTYPES_STATE_INITIALIZED)
    {
        result = __LINE__;
    }
    else
    {
        char* normalized_type = umocktypename_normalize(type);
        if (normalized_type == NULL)
        {
            /* Codes_SRS_UMOCKTYPES_01_045: [ If normalizing the typename fails, umocktypes_register_type shall fail and return a non-zero value. ]*/
            result = __LINE__;
        }
        else
        {
            UMOCK_VALUE_TYPE_HANDLERS* type_handler = get_value_type_handlers(normalized_type);
            if (type_handler != NULL)
            {
                free(normalized_type);

                if ((stringify_func != type_handler->stringify_func) ||
                    (are_equal_func != type_handler->are_equal_func) ||
                    (copy_func != type_handler->copy_func) ||
                    (free_func != type_handler->free_func))
                {
                    /* Codes_SRS_UMOCKTYPES_01_011: [ If the type has already been registered but at least one of the function pointers is different, umocktypes_register_type shall fail and return a non-zero value. ]*/
                    result = __LINE__;
                }
                else
                {
                    /* Codes_SRS_UMOCKTYPES_01_010: [ If the type has already been registered with the same function pointers then umocktypes_register_type shall succeed and return 0. ]*/
                    result = 0;
                }
            }
            else
            {
                UMOCK_VALUE_TYPE_HANDLERS* new_type_handlers = (UMOCK_VALUE_TYPE_HANDLERS*)realloc(type_handlers, sizeof(UMOCK_VALUE_TYPE_HANDLERS) * (type_handler_count + 1));
                if (new_type_handlers == NULL)
                {
                    /* Codes_SRS_UMOCKTYPES_01_012: [ If an error occurs allocating memory for the newly registered type, umocktypes_register_type shall fail and return a non-zero value. ]*/
                    free(normalized_type);
                    result = __LINE__;
                }
                else
                {
                    /* Codes_SRS_UMOCKTYPES_01_007: [ umocktypes_register_type shall register an interface made out of the stringify, are equal, copy and free functions for the type identified by the argument type. ] */
                    type_handlers = new_type_handlers;
                    type_handlers[type_handler_count].type = normalized_type;
                    type_handlers[type_handler_count].stringify_func = stringify_func;
                    type_handlers[type_handler_count].copy_func = copy_func;
                    type_handlers[type_handler_count].free_func = free_func;
                    type_handlers[type_handler_count].are_equal_func = are_equal_func;
                    type_handler_count++;

                    /* Codes_SRS_UMOCKTYPES_01_008: [ On success umocktypes_register_type shall return 0. ]*/
                    result = 0;
                }
            }
        }
    }

    return result;
}

int umocktypes_register_alias_type(const char* type, const char* is_type)
{
    int result;

    if ((type == NULL) || (is_type == NULL))
    {
        result = __LINE__;
    }
    else if (umocktypes_state != UMOCKTYPES_STATE_INITIALIZED)
    {
        result = __LINE__;
    }
    else
    {
        char* normalized_is_type = umocktypename_normalize(is_type);
        if (normalized_is_type == NULL)
        {
            result = __LINE__;
        }
        else
        {
            UMOCK_VALUE_TYPE_HANDLERS* value_type_handlers = get_value_type_handlers(normalized_is_type);
            if (value_type_handlers == NULL)
            {
                result = __LINE__;
            }
            else
            {
                char* normalized_type = umocktypename_normalize(type);
                if (normalized_type == NULL)
                {
                    result = __LINE__;
                }
                else
                {
                    if (strcmp(normalized_type, normalized_is_type) == 0)
                    {
                        free(normalized_type);

                        result = 0;
                    }
                    else
                    {
                        UMOCK_VALUE_TYPE_HANDLERS* new_type_handlers = (UMOCK_VALUE_TYPE_HANDLERS*)realloc(type_handlers, sizeof(UMOCK_VALUE_TYPE_HANDLERS) * (type_handler_count + 1));
                        if (new_type_handlers == NULL)
                        {
                            free(normalized_type);
                            result = __LINE__;
                        }
                        else
                        {
                            type_handlers = new_type_handlers;
                            type_handlers[type_handler_count].type = normalized_type;
                            type_handlers[type_handler_count].stringify_func = value_type_handlers->stringify_func;
                            type_handlers[type_handler_count].copy_func = value_type_handlers->copy_func;
                            type_handlers[type_handler_count].free_func = value_type_handlers->free_func;
                            type_handlers[type_handler_count].are_equal_func = value_type_handlers->are_equal_func;
                            type_handler_count++;

                            result = 0;
                        }
                    }
                }
            }

            free(normalized_is_type);
        }
    }

    return result;
}

/* Codes_SRS_UMOCKTYPES_01_013: [ umocktypes_stringify shall return a char\* with the string representation of the value argument. ]*/
char* umocktypes_stringify(const char* type, const void* value)
{
    char* result;

    if ((type == NULL) ||
        (value == NULL))
    {
        /* Codes_SRS_UMOCKTYPES_01_016: [ If any of the arguments is NULL, umocktypes_stringify shall fail and return NULL. ]*/
        result = NULL;
    }
    /* Codes_SRS_UMOCKTYPES_01_049: [ If umocktypes_stringify is called when the module is not initialized, umocktypes_stringify shall return NULL. ]*/
    else if (umocktypes_state != UMOCKTYPES_STATE_INITIALIZED)
    {
        result = NULL;
    }
    else
    {
        /* Codes_SRS_UMOCKTYPES_01_035: [ Before looking it up, the type string shall be normalized by calling umocktypename_normalize. ]*/
        char* normalized_type = umocktypename_normalize(type);
        if (normalized_type == NULL)
        {
            /* Codes_SRS_UMOCKTYPES_01_044: [ If normalizing the typename fails, umocktypes_stringify shall fail and return NULL. ]*/
            result = NULL;
        }
        else
        {
            UMOCK_VALUE_TYPE_HANDLERS* value_type_handlers = get_value_type_handlers(normalized_type);
            if (value_type_handlers == NULL)
            {
                /* Codes_SRS_UMOCKTYPES_01_017: [ If type can not be found in the registered types list maintained by the module, umocktypes_stringify shall fail and return NULL. ]*/
                result = NULL;
            }
            else
            {
                /* Codes_SRS_UMOCKTYPES_01_014: [ The string representation shall be obtained by calling the stringify function registered for the type identified by the argument type. ]*/
                /* Codes_SRS_UMOCKTYPES_01_015: [ On success umocktypes_stringify shall return the char\* produced by the underlying stringify function for type (passed in umocktypes_register_type). ]*/
                result = value_type_handlers->stringify_func(value);
            }

            free(normalized_type);
        }
    }

    return result;
}

/* Codes_SRS_UMOCKTYPES_01_018: [ umocktypes_are_equal shall evaluate whether 2 values are equal. ]*/
int umocktypes_are_equal(const char* type, const void* left, const void* right)
{
    int result;

    if ((type == NULL) ||
        (left == NULL) ||
        (right == NULL))
    {
        /* Codes_SRS_UMOCKTYPES_01_023: [ If any of the arguments is NULL, umocktypes_are_equal shall fail and return -1. ]*/
        result = -1;
    }
    else if(umocktypes_state != UMOCKTYPES_STATE_INITIALIZED)
    {
        /* Codes_SRS_UMOCKTYPES_01_046: [ If umocktypes_are_equal is called when the module is not initialized, umocktypes_are_equal shall return -1. ] */
        result = -1;
    }
    else
    {
        /* Codes_SRS_UMOCKTYPES_01_036: [ Before looking it up, the type string shall be normalized by calling umocktypename_normalize. ]*/
        char* normalized_type = umocktypename_normalize(type);
        if (normalized_type == NULL)
        {
            /* Codes_SRS_UMOCKTYPES_01_043: [ If normalizing the typename fails, umocktypes_are_equal shall fail and return -1. ]*/
            result = -1;
        }
        else
        {
            UMOCK_VALUE_TYPE_HANDLERS* value_type_handlers = get_value_type_handlers(normalized_type);
            if (value_type_handlers == NULL)
            {
                /* Codes_SRS_UMOCKTYPES_01_024: [ If type can not be found in the registered types list maintained by the module, umocktypes_are_equal shall fail and return -1. ]*/
                result = -1;
            }
            else
            {
                /* Codes_SRS_UMOCKTYPES_01_051: [ If the pointer values for left and right are equal, umocktypes_are_equal shall return 1 without calling the underlying are_equal function. ]*/
                if (left == right)
                {
                    result = 1;
                }
                else
                {
                    /* Codes_SRS_UMOCKTYPES_01_019: [ umocktypes_are_equal shall call the underlying are_equal function for the type identified by the argument type (passed in umocktypes_register_type). ] */
                    switch (value_type_handlers->are_equal_func(left, right))
                    {
                    default:
                        /* Codes_SRS_UMOCKTYPES_01_020: [ If the underlying are_equal function fails,, umocktypes_are_equal shall fail and return -1. ] */
                        result = -1;
                        break;

                    case 1:
                        /* Codes_SRS_UMOCKTYPES_01_021: [ If the underlying are_equal function indicates the types are equal, umocktypes_are_equal shall return 1. ]*/
                        result = 1;
                        break;

                    case 0:
                        /* Codes_SRS_UMOCKTYPES_01_022: [ If the underlying are_equal function indicates the types are not equal, umocktypes_are_equal shall return 0. ]*/
                        result = 0;
                        break;
                    }
                }
            }

            free(normalized_type);
        }
    }

    return result;
}

/* Codes_SRS_UMOCKTYPES_01_025: [ umocktypes_copy shall copy the value of the source into the destination argument. ]*/
int umocktypes_copy(const char* type, void* destination, const void* source)
{
    int result;

    if ((type == NULL) ||
        (destination == NULL) ||
        (source == NULL))
    {
        /* Codes_SRS_UMOCKTYPES_01_027: [ If any of the arguments is NULL, umocktypes_copy shall return -1. ]*/
        result = -1;
    }
    else if (umocktypes_state != UMOCKTYPES_STATE_INITIALIZED)
    {
        /* Codes_SRS_UMOCKTYPES_01_047: [ If umocktypes_copy is called when the module is not initialized, umocktypes_copy shall fail and return a non zero value. ]*/
        result = -1;
    }
    else
    {
        /* Codes_SRS_UMOCKTYPES_01_037: [ Before looking it up, the type string shall be normalized by calling umocktypename_normalize. ]*/
        char* normalized_type = umocktypename_normalize(type);
        if (normalized_type == NULL)
        {
            /* Codes_SRS_UMOCKTYPES_01_042: [ If normalizing the typename fails, umocktypes_copy shall fail and return a non-zero value. ]*/
            result = -1;
        }
        else
        {
            UMOCK_VALUE_TYPE_HANDLERS* value_type_handlers = get_value_type_handlers(normalized_type);
            if (value_type_handlers == NULL)
            {
                /* Codes_SRS_UMOCKTYPES_01_029: [ If type can not be found in the registered types list maintained by the module, umocktypes_copy shall fail and return -1. ]*/
                result = __LINE__;
            }
            else
            {
                /* Codes_SRS_UMOCKTYPES_01_026: [ The copy shall be done by calling the underlying copy function (passed in umocktypes_register_type) for the type identified by the type argument. ]*/
                /* Codes_SRS_UMOCKTYPES_01_052: [ On success, umocktypes_copy shall return 0. ]*/
                /* Codes_SRS_UMOCKTYPES_01_028: [ If the underlying copy fails, umocktypes_copy shall return -1. ]*/
                result = value_type_handlers->copy_func(destination, source);
            }

            free(normalized_type);
        }
    }

    return result;
}

/* Codes_SRS_UMOCKTYPES_01_030: [ umocktypes_free shall free a value previously allocated with umocktypes_copy. ]*/
void umocktypes_free(const char* type, void* value)
{
    /* Codes_SRS_UMOCKTYPES_01_031: [ If any of the arguments is NULL, umocktypes_free shall do nothing. ]*/
    if ((type != NULL) &&
        (value != NULL) &&
        /* Codes_SRS_UMOCKTYPES_01_048: [ If umocktypes_free is called when the module is not initialized, umocktypes_free shall do nothing. ]*/
        (umocktypes_state == UMOCKTYPES_STATE_INITIALIZED))
    {
        /* Codes_SRS_UMOCKTYPES_01_038: [ Before looking it up, the type string shall be normalized by calling umocktypename_normalize. ]*/
        char* normalized_type = umocktypename_normalize(type);
        /* Codes_SRS_UMOCKTYPES_01_032: [ If type can not be found in the registered types list maintained by the module, umocktypes_free shall do nothing. ]*/
        if (normalized_type != NULL)
        {
            UMOCK_VALUE_TYPE_HANDLERS* value_type_handlers = get_value_type_handlers(normalized_type);
            if (value_type_handlers != NULL)
            {
                /* Codes_SRS_UMOCKTYPES_01_033: [ The free shall be done by calling the underlying free function (passed in umocktypes_register_type) for the type identified by the type argument. ]*/
                value_type_handlers->free_func(value);
            }

            free(normalized_type);
        }
    }
}
