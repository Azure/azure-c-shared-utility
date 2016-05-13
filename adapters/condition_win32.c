// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdlib.h>

#include "azure_c_shared_utility/condition.h"
#include "windows.h"
#include "azure_c_shared_utility/iot_logging.h"
#include "azure_c_shared_utility/gballoc.h"

DEFINE_ENUM_STRINGS(COND_RESULT, COND_RESULT_VALUES);

typedef struct CONDITION_TAG
{
    int waiting_thread_count;
    CRITICAL_SECTION count_lock;
    HANDLE event_handle;
}
CONDITION;

COND_HANDLE Condition_Init(void)
{
    // Codes_SRS_CONDITION_18_002: [ Condition_Init shall create and return a CONDITION_HANDLE ]

    CONDITION* cond = (CONDITION*)malloc(sizeof(CONDITION));

    // Codes_SRS_CONDITION_18_008: [ Condition_Init shall return NULL if it fails to allocate the CONDITION_HANDLE ]
    if (cond != NULL)
    {
        cond->event_handle = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (cond->event_handle == INVALID_HANDLE_VALUE)
        {
            free(cond);
            cond = NULL;
        }
        else
        {
            /* Needed to emulate pthread_signal as we only signal the event when there are waiting threads */
            cond->waiting_thread_count = 0;
            InitializeCriticalSection(&cond->count_lock);
        }
    }

    return (COND_HANDLE)cond;
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
        CONDITION* cond = (CONDITION*)handle;
        int waiting_thread_count;

        EnterCriticalSection(&cond->count_lock);
        waiting_thread_count = cond->waiting_thread_count;
        LeaveCriticalSection(&cond->count_lock);

        if (waiting_thread_count == 0 || SetEvent(cond->event_handle))
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
        CONDITION* cond = (CONDITION*)handle;
        DWORD wait_result;

        if (timeout_milliseconds == 0)
        {
            timeout_milliseconds = INFINITE;
        }

        EnterCriticalSection(&cond->count_lock);
        cond->waiting_thread_count++;
        LeaveCriticalSection(&cond->count_lock);

        Unlock(lock);

        // Codes_SRS_CONDITION_18_013: [ Condition_Wait shall accept relative timeouts ]
        wait_result = WaitForSingleObject(cond->event_handle, timeout_milliseconds);

        Lock(lock);

        if (wait_result != WAIT_OBJECT_0 || wait_result != WAIT_TIMEOUT)
        {
            /* cond might be freed at this point, just return error */
            result = COND_ERROR;
        }
        else
        {
            EnterCriticalSection(&cond->count_lock);
            cond->waiting_thread_count--;
            LeaveCriticalSection(&cond->count_lock);

            if (wait_result == WAIT_TIMEOUT)
            {
                // Codes_SRS_CONDITION_18_011: [ Condition_Wait shall return COND_TIMEOUT if the condition is NOT triggered and timeout_milliseconds is not 0 ]
                result = COND_TIMEOUT;
            }
            else 
            {
                // Codes_SRS_CONDITION_18_012: [ Condition_Wait shall return COND_OK if the condition is triggered and timeout_milliseconds is not 0 ]
                result = COND_OK;
            }
        }
    }
    return result;
}

void Condition_Deinit(COND_HANDLE handle)
{
    // Codes_SRS_CONDITION_18_007: [ Condition_Deinit will not fail if handle is NULL ]
    // Codes_SRS_CONDITION_18_009: [ Condition_Deinit will deallocate handle if it is not NULL 
    if (handle != NULL)
    {
        CONDITION* cond = (CONDITION*)handle;

        (void)CloseHandle(cond->event_handle);
        cond->event_handle = INVALID_HANDLE_VALUE;

        DeleteCriticalSection(&cond->count_lock);
        cond->waiting_thread_count = 0;

        free(cond);
    }
}

