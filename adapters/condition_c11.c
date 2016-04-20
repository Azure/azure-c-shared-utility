// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdlib.h>

#include "azure_c_shared_utility/condition.h"
#include "azure_c_shared_utility/iot_logging.h"
#include <thr/threads.h>
#include "azure_c_shared_utility/gballoc.h"

DEFINE_ENUM_STRINGS(COND_RESULT, COND_RESULT_VALUES);

COND_HANDLE Condition_Init(void)
{
    // Codes_SRS_CONDITION_18_002: [ Condition_Init shall create and return a CONDITION_HANDLE ]
    cnd_t* cond = (cnd_t*)malloc(sizeof(cnd_t));
    if (cond != NULL)
    {
        cnd_init(cond);
    }
    // Codes_SRS_CONDITION_18_008: [ Condition_Init shall return NULL if it fails to allocate the CONDITION_HANDLE ]
    return cond;
}

COND_RESULT Condition_Post(COND_HANDLE handle)
{
    COND_RESULT result;
    if (handle == NULL)
    {
        // Codes_SRS_CONDITION_18_001: [ Condition_Post shall return COND_INVALID_ARG if handle is NULL ]
        result = COND_INVALID_ARG;
    }
    else
    {
        if (cnd_broadcast((cnd_t*)handle) == thrd_success)
        {
            // Codes_SRS_CONDITION_18_003: [ Condition_Post shall return COND_OK if it succcessfully posts the condition ]
            result = COND_OK;
        }
        else
        {
            result = COND_ERROR;
        }
    }
    return result;
}

COND_RESULT Condition_Wait(COND_HANDLE handle, LOCK_HANDLE lock, int timeout_milliseconds)
{
    COND_RESULT result;
    // Codes_SRS_CONDITION_18_004: [ Condition_Wait shall return COND_INVALID_ARG if handle is NULL ]
    // Codes_SRS_CONDITION_18_005: [ Condition_Wait shall return COND_INVALID_ARG if lock is NULL and timeout_milliseconds is 0 ]
    // Codes_SRS_CONDITION_18_006: [ Condition_Wait shall return COND_INVALID_ARG if lock is NULL and timeout_milliseconds is not 0 ]
    if (handle == NULL || lock == NULL)
    {
        result = COND_INVALID_ARG;
    }
    else
    {
        if (timeout_milliseconds > 0)
        {
            struct xtime tm;
            int wait_result;
            xtime_get(&tm, TIME_UTC);

            // Codes_SRS_CONDITION_18_013: [ Condition_Wait shall accept relative timeouts ]
            tm.sec += (timeout_milliseconds / 1000);
            tm.nsec += (timeout_milliseconds % 1000) * 1000000L;

            wait_result = cnd_timedwait((cnd_t *)handle, (mtx_t*)lock, &tm);
            if (wait_result == thrd_timedout)
            {
                // Codes_SRS_CONDITION_18_011: [ Condition_Wait shall return COND_TIMEOUT if the condition is NOT triggered and timeout_milliseconds is not 0 ]
                result = COND_TIMEOUT;
            }
            else if (wait_result == thrd_success)
            {
                // Codes_SRS_CONDITION_18_012: [ Condition_Wait shall return COND_OK if the condition is triggered and timeout_milliseconds is not 0 ]
                result = COND_OK;
            }
            else
            {
                LogError("Failed to Condition_Wait");
                result = COND_ERROR;
            }
        }
        else
        {
            if (cnd_wait((cnd_t*)handle, (mtx_t *)lock) != thrd_success)
            {
                LogError("Failed to cnd_wait");
                result = COND_ERROR;
            }
            else
            {
                // Codes_SRS_CONDITION_18_010: [ Condition_Wait shall return COND_OK if the condition is triggered and timeout_milliseconds is 0 ]
                result = COND_OK;
            }
        }
    }
    return result;
}

void Condition_Deinit(COND_HANDLE handle)
{
    // Codes_SRS_CONDITION_18_007: [ Condition_Deinit will not fail if handle is NULL ]
    if (handle != NULL)
    {
        // Codes_SRS_CONDITION_18_009: [ Condition_Deinit will deallocate handle if it is not NULL 
        cnd_t* cond = (cnd_t*)handle;
        cnd_destroy(cond);
        free(cond);
    }
}

