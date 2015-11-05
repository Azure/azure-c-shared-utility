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
#include <threads.h>

DEFINE_ENUM_STRINGS(COND_RESULT, COND_RESULT_VALUES);

COND_HANDLE Condition_Init(void)
{
    cnd_t* cond = (cnd_t*)malloc(sizeof(cnd_t));
    cnd_init(cond);
    return cond;
}

COND_RESULT Condition_Post(COND_HANDLE handle)
{
    cnd_broadcast((cnd_t*)handle);
    return COND_OK;
}

COND_RESULT Condition_Wait(COND_HANDLE  handle, LOCK_HANDLE lock, int timeout_milliseconds)
{
    if (timeout_milliseconds > 0)
    {
        struct xtime tm;
        tm.sec = timeout_milliseconds / 1000;
        tm.nsec = (timeout_milliseconds % 1000) * 1000000L;
        int wait_result = cnd_timedwait((cnd_t *)handle, (mtx_t *)lock, &tm);
        if (wait_result == ETIMEDOUT)
        {
            return COND_TIMEOUT;
        }
        else
        {
            LogError("Failed to Condition_Wait\r\n");
            return COND_ERROR;
        }
    }
    else
    {
        if (cnd_wait((cnd_t*)handle, (mtx_t *)lock) != 0)
        {
            LogError("Failed to cnd_wait\r\n");
            return COND_ERROR;
        }
    }
    return COND_OK;
}

COND_RESULT Condition_Deinit(COND_HANDLE  handle)
{
    cnd_destroy((cnd_t*)handle);
    return COND_OK;
}
