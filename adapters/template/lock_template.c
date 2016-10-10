// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/lock.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/macro_utils.h"

/* This is a template file used for porting */

/* Please go through all the TODO sections below and implement the needed code */

DEFINE_ENUM_STRINGS(LOCK_RESULT, LOCK_RESULT_VALUES);

LOCK_HANDLE Lock_Init(void)
{
	/* TODO: Call a function that creates a lack (mutes) here and returns a handle to it 
	void* my_lock = a_function_that_creates_a_lock();
    return (LOCK_HANDLE) ...my_lock...;*/
}

LOCK_RESULT Lock(LOCK_HANDLE handle)
{
    LOCK_RESULT result = LOCK_OK;
    if (handle == NULL)
    {
        result = LOCK_ERROR;
        LogError("(result = %s)", ENUM_TO_STRING(LOCK_RESULT, result));
    }
    else 
    {
		/* TODO: Call a function that takes the lock
		take_a_lock(handle);*/
    }    
    return result;
}

LOCK_RESULT Unlock(LOCK_HANDLE handle)
{
    LOCK_RESULT result = LOCK_OK;
    if (handle == NULL)
    {
        result = LOCK_ERROR;
        LogError("(result = %s)", ENUM_TO_STRING(LOCK_RESULT, result));
    }
    else
    {
		/* TODO: Call a function that releases the lock
		release_a_lock(handle);*/
    }
    
    return result;
}

LOCK_RESULT Lock_Deinit(LOCK_HANDLE handle)
{
    LOCK_RESULT result = LOCK_OK;
    if (handle == NULL)
    {
        /*SRS_LOCK_99_007:[ This API on NULL handle passed returns LOCK_ERROR]*/
        result = LOCK_ERROR;
        LogError("(result = %s)", ENUM_TO_STRING(LOCK_RESULT, result));
    }
    else
    {
		/* TODO: Call a function that frees the lock
		free_a_lock(handle);*/
    }
    return result;
}