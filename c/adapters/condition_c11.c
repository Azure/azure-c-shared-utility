// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "stdlib.h"

#include "macro_utils.h"
#include "condition.h"
#include "iot_logging.h"
#include <thr/threads.h>

DEFINE_ENUM_STRINGS(COND_RESULT, COND_RESULT_VALUES);

COND_HANDLE Condition_Init(void)
{
    cnd_t* cond = (cnd_t*)malloc(sizeof(cnd_t));
    if (cond != NULL)
    {
        cnd_init(cond);
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
        if (cnd_broadcast((cnd_t*)handle) == thrd_success)
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
            struct xtime tm;
            int wait_result;
            time_t now = get_time(NULL);

            tm.sec = (unsigned long)get_difftime(now, (time_t)0) + (timeout_milliseconds / 1000);
            tm.nsec = (timeout_milliseconds % 1000) * 1000000L;
            wait_result = cnd_timedwait((cnd_t *)handle, (mtx_t*)lock, &tm);
            if (wait_result == thrd_timedout)
            {
                result = COND_TIMEOUT;
            }
            else if (wait_result == thrd_success)
            {
                result = COND_OK;
            }
            else
            {
                LogError("Failed to Condition_Wait\r\n");
                result = COND_ERROR;
            }
        }
        else
        {
            if (cnd_wait((cnd_t*)handle, (mtx_t *)lock) != thrd_success)
            {
                LogError("Failed to cnd_wait\r\n");
                result = COND_ERROR;
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
        cnd_t* cond = (cnd_t*)handle;
        cnd_destroy(cond);
        free(cond);
    }
}
