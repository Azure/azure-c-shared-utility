// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/* This is a template file used for porting */

#include <stdlib.h>
#include <stdint.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

typedef struct TICK_COUNTER_INSTANCE_TAG
{
	uint32_t create_time;
} TICK_COUNTER_INSTANCE;

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    TICK_COUNTER_INSTANCE* result = (TICK_COUNTER_INSTANCE*)malloc(sizeof(TICK_COUNTER_INSTANCE));
    if (result == NULL)
    {
		/* add any per instance initialization (starting a timer for example) here if needed (most platforms will not need this) */

		result->create_time = /*  Save the initial tick value at tickcounter_create time */
		
		LogError("Cannot create tick counter");
    }
	
    return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    if (tick_counter != NULL)
    {
		/* add any per instance de-initialization here (stopping the time) if needed (most platforms will not need this) */
		
        free(tick_counter);
    }
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, uint64_t* current_ms)
{
    int result;
	
    if (tick_counter == NULL)
    {
        LogError("tickcounter failed: Invalid Arguments.");
        result = __FAILURE__;
    }
    else
    {
		/* call here any platform/OS specific function to get the millisecond tick and perform any conversions */
		
		*current_ms = /* return the difference in milliseconds between the time at tickcounter_create and now */;
    }
	
    return result;
}
