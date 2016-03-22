// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdio.h>

#include "umock_c.h"
#include "umockcall.h"
#include "umocktypes.h"
#include "umocktypes_c.h"
#include "umockcallrecorder.h"

typedef enum UMOCK_C_STATE_TAG
{
    UMOCK_C_STATE_NOT_INITIALIZED,
    UMOCK_C_STATE_INITIALIZED
} UMOCK_C_STATE;

static ON_UMOCK_C_ERROR on_umock_c_error_function;
static UMOCK_C_STATE umock_c_state = UMOCK_C_STATE_NOT_INITIALIZED;
static UMOCKCALLRECORDER_HANDLE call_recorder = NULL;

int umock_c_init(ON_UMOCK_C_ERROR on_umock_c_error)
{
    int result;

    if (umock_c_state != UMOCK_C_STATE_NOT_INITIALIZED)
    {
        result = __LINE__;
    }
    else
    {
        /* Codes_SRS_UMOCK_C_LIB_01_144: [ Out of the box umock_c shall support the following types through the header umocktypes_c.h: ]*/
        /* Codes_SRS_UMOCK_C_LIB_01_028: [**char**] */
        /* Codes_SRS_UMOCK_C_LIB_01_029 : [**unsigned char**] */
        /* Codes_SRS_UMOCK_C_LIB_01_030 : [**short**] */
        /* Codes_SRS_UMOCK_C_LIB_01_031 : [**unsigned short**] */
        /* Codes_SRS_UMOCK_C_LIB_01_032 : [**int**] */
        /* Codes_SRS_UMOCK_C_LIB_01_033 : [**unsigned int**] */
        /* Codes_SRS_UMOCK_C_LIB_01_034 : [**long**] */
        /* Codes_SRS_UMOCK_C_LIB_01_035 : [**unsigned long**] */
        /* Codes_SRS_UMOCK_C_LIB_01_036 : [**long long**] */
        /* Codes_SRS_UMOCK_C_LIB_01_037 : [**unsigned long long**] */
        /* Codes_SRS_UMOCK_C_LIB_01_038 : [**float**] */
        /* Codes_SRS_UMOCK_C_LIB_01_039 : [**double**] */
        /* Codes_SRS_UMOCK_C_LIB_01_040 : [**long double**] */
        /* Codes_SRS_UMOCK_C_LIB_01_041 : [**size_t**] */
        if ((umocktypes_init() != 0) ||
            (umocktypes_c_register_types() != 0))
        {
            result = __LINE__;
        }
        else
        {
            call_recorder = umockcallrecorder_create();
            on_umock_c_error_function = on_umock_c_error;
            umock_c_state = UMOCK_C_STATE_INITIALIZED;
            result = 0;
        }
    }

    return result;
}

void umock_c_deinit(void)
{
    /* Codes_SRS_UMOCK_C_LIB_01_012: [If umock_c was not initialized, umock_c_deinit shall do nothing.] */
    /* Codes_SRS_UMOCK_C_01_010: [ If the module is not initialized, umock_c_deinit shall do nothing. ] */
    if (umock_c_state == UMOCK_C_STATE_INITIALIZED)
    {
        umockcallrecorder_destroy(call_recorder);
        umocktypes_deinit();

        umock_c_state = UMOCK_C_STATE_NOT_INITIALIZED;
    }
}

void umock_c_indicate_error(UMOCK_C_ERROR_CODE error_code)
{
    if (on_umock_c_error_function != NULL)
    {
        on_umock_c_error_function(error_code);
    }
}

void umock_c_reset_all_calls(void)
{
    /* Codes_SRS_UMOCK_C_01_012: [ If the module is not initialized, umock_c_reset_all_calls shall do nothing. ]*/
    if (umock_c_state == UMOCK_C_STATE_INITIALIZED)
    {
        if (umockcallrecorder_reset_all_calls(call_recorder) != 0)
        {
            umock_c_indicate_error(UMOCK_C_ERROR);
        }
    }
}

int umock_c_add_expected_call(UMOCKCALL_HANDLE mock_call)
{
    int result;

    if (umock_c_state != UMOCK_C_STATE_INITIALIZED)
    {
        result = __LINE__;
    }
    else
    {
        result = umockcallrecorder_add_expected_call(call_recorder, mock_call);
    }

    return result;
}

int umock_c_add_actual_call(UMOCKCALL_HANDLE mock_call, UMOCKCALL_HANDLE* matched_call)
{
    int result;

    if (umock_c_state != UMOCK_C_STATE_INITIALIZED)
    {
        result = __LINE__;
    }
    else
    {
        result = umockcallrecorder_add_actual_call(call_recorder, mock_call, matched_call);
    }

    return result;
}

const char* umock_c_get_expected_calls(void)
{
    const char* result;

    if (umock_c_state != UMOCK_C_STATE_INITIALIZED)
    {
        result = NULL;
    }
    else
    {
        result = umockcallrecorder_get_expected_calls(call_recorder);
    }

    return result;
}

const char* umock_c_get_actual_calls(void)
{
    const char* result;

    if (umock_c_state != UMOCK_C_STATE_INITIALIZED)
    {
        result = NULL;
    }
    else
    {
        result = umockcallrecorder_get_actual_calls(call_recorder);
    }

    return result;
}

UMOCKCALL_HANDLE umock_c_get_last_expected_call(void)
{
    UMOCKCALL_HANDLE result;

    if (umock_c_state != UMOCK_C_STATE_INITIALIZED)
    {
        result = NULL;
    }
    else
    {
        result = umockcallrecorder_get_last_expected_call(call_recorder);
    }

    return result;
}
