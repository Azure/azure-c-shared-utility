// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "lock.h"
#include <stdlib.h>
#include "condition.h"
#include <iot_logging.h>
#include <errno.h>
#include <pthread.h>

DEFINE_ENUM_STRINGS(COND_RESULT, COND_RESULT_VALUES);

COND_HANDLE Condition_Init(void)
{
    pthread_cond_t * cond = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
    if (cond != NULL)
    {
        pthread_cond_init(cond, NULL);
    }
    return cond;
}

COND_RESULT Condition_Post(COND_HANDLE handle)
{
    COND_RESULT result;
    if (handle == NULL)
    {
        result = COND_INVALID_ARG;
    }
    else
    {
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

COND_RESULT Condition_Wait(COND_HANDLE handle, LOCK_HANDLE lock, int timeout_milliseconds)
{
    COND_RESULT result;
    if (handle == NULL || lock == NULL)
    {
        result = COND_INVALID_ARG;
    }
    else
    {
        if (timeout_milliseconds > 0)
        {
            struct timespec tm;
            tm.tv_sec = timeout_milliseconds / 1000;
            tm.tv_nsec = (timeout_milliseconds % 1000) * 1000000L;
            int wait_result = pthread_cond_timedwait((pthread_cond_t *)handle, (pthread_mutex_t *)lock, &tm);
            if (wait_result == ETIMEDOUT)
            {
                return COND_TIMEOUT;
            }
            else if (wait_result == 0)
            {
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
                result = COND_OK;
            }
        }
    }
    return result;
}

void Condition_Deinit(COND_HANDLE handle)
{
    if (handle != NULL)
    {
        pthread_cond_t* cond = (pthread_cond_t*)handle;
        pthread_cond_destroy(cond);
        free(cond);
    }
}
