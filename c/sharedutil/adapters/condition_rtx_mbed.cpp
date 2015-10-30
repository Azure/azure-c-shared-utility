// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <cstdlib>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "lock.h"
#include "iot_logging.h"
#include "rtos.h"

COND_HANDLE Condition_Init(void)
{
    return NULL;
}

COND_RESULT Condition_Post(COND_HANDLE handle)
{
    return COND_ERROR;
}

COND_RESULT Condition_Wait(COND_HANDLE  handle, LOCK_HANDLE lock, int timeout_milliseconds)
{
    return COND_ERROR;
}

COND_RESULT Condition_Deinit(COND_HANDLE  handle)
{
    return COND_ERROR;
}
