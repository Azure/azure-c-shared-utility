// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdint.h>
#include "gballoc.h"
#include "tickcounter.h"
#include "xlogging.h"
#include "mico.h"

typedef struct TICK_COUNTER_INSTANCE_TAG
{
    uint32_t dummy : 1;
} TICK_COUNTER_INSTANCE;

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    TICK_COUNTER_INSTANCE* result = (TICK_COUNTER_INSTANCE*)malloc(sizeof(TICK_COUNTER_INSTANCE));
    if (result == NULL)
    {
		LogError("Cannot create tick counter");
    }
	
    return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    if (tick_counter != NULL)
    {
        free(tick_counter);
    }
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, tickcounter_ms_t* current_ms)
{
    int result;
	
    if (tick_counter == NULL)
    {
        LogError("tickcounter failed: Invalid Arguments.");
        result = __LINE__;
    }
    else
    {
        mico_time_t currentTime = mico_rtos_get_time();
		*current_ms = currentTime;
		result = 0;
    }
	
    return result;
}
