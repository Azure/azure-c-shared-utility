// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "lock.h"
#include "condition.h"
#include "iot_logging.h"
#include <errno.h>
#include <pthread.h>
#include "gballoc.h"

DEFINE_ENUM_STRINGS(COND_RESULT, COND_RESULT_VALUES);

COND_HANDLE Condition_Init(void)
{
    // Codes_SRS_CONDITION_18_002: [ Condition_Init shall create and return a CONDITION_HANDLE ]
    pthread_cond_t * cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    if (cond != NULL)
    {
        pthread_cond_init(cond, NULL);
    }
    // Codes_SRS_CONDITION_18_008: [ Condition_Init shall return NULL if it fails to allocate the CONDITION_HANDLE ]
    return cond;
}

COND_RESULT Condition_Post(COND_HANDLE handle)
{
    COND_RESULT result;
    // Codes_SRS_CONDITION_18_001: [ Condition_Post shall return COND_INVALID_ARG if handle is NULL ]
    if (handle == NULL)
    {
        result = COND_INVALID_ARG;
    }
    else
    {
        // Codes_SRS_CONDITION_18_003: [ Condition_Post shall return COND_OK if it succcessfully posts the condition ]
        if (pthread_cond_broadcast((pthread_cond_t*)handle) == 0)
        {
            result = COND_OK;
        }
        else
        {
            result = COND_ERROR;
        }
    }
    return result;
}


#define NANOSECONDS_IN_1_SECOND 1000000000L
#define MILLISECONDS_IN_1_SECOND 1000
#define NANOSECONDS_IN_1_MILLISECOND 1000000L
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
            struct timespec tm;
            clock_gettime(CLOCK_REALTIME,&tm);
            tm.tv_nsec += (timeout_milliseconds % MILLISECONDS_IN_1_SECOND) * NANOSECONDS_IN_1_MILLISECOND;
            tm.tv_sec += timeout_milliseconds / MILLISECONDS_IN_1_SECOND;
            // handle overflow in tv_nsec
            tm.tv_sec+= tm.tv_nsec / NANOSECONDS_IN_1_SECOND;
            tm.tv_nsec %= NANOSECONDS_IN_1_SECOND;
            int wait_result = pthread_cond_timedwait((pthread_cond_t *)handle, (pthread_mutex_t *)lock, &tm);
            if (wait_result == ETIMEDOUT)
            {
                // Codes_SRS_CONDITION_18_011: [ Condition_Wait shall return COND_TIMEOUT if the condition is triggered and timeout_milliseconds is not 0 ]
                return COND_TIMEOUT;
            }
            else if (wait_result == 0)
            {
                // Codes_SRS_CONDITION_18_012: [ Condition_Wait shall return COND_OK if the condition is triggered and timeout_milliseconds is not 0 ]
                result = COND_OK;
            }
            else
            {
                LogError("Failed to pthread_cond_timedwait\r\n");
                return COND_ERROR;
            }
        }
        else
        {
            if (pthread_cond_wait((pthread_cond_t*)handle, (pthread_mutex_t *)lock) != 0)
            {
                LogError("Failed to pthread_cond_wait\r\n");
                return COND_ERROR;
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
        pthread_cond_t* cond = (pthread_cond_t*)handle;
        pthread_cond_destroy(cond);
        free(cond);
    }
}


