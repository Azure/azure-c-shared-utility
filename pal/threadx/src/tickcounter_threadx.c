// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Copyright (c) Express Logic.  All rights reserved.
// Please contact support@expresslogic.com for any questions or use the support portal at www.rtos.com

/* This file is used for porting tickcounter between threadx and azure-iot-sdk-c.  */

#include <stdlib.h>
#include <stdint.h>
#include "tx_api.h"
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/tickcounter.h"
#include "azure_c_shared_utility/optimize_size.h"
#include "azure_c_shared_utility/xlogging.h"

typedef struct TICK_COUNTER_INSTANCE_TAG
{
    UINT    timer_clock;
} TICK_COUNTER_INSTANCE;

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    TICK_COUNTER_INSTANCE* result = (TICK_COUNTER_INSTANCE*)malloc(sizeof(TICK_COUNTER_INSTANCE));
    if (result != NULL)
    {

        /* Get the timer clock (ticks) for creating.  */
        result -> timer_clock = tx_time_get();
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
    int     result = 0;
    UINT    current_timer_clock;

    if (tick_counter == NULL || current_ms == NULL)
    {
        result = MU_FAILURE;
    }
    else
    {
        /* Get the current timer clock.  */
        current_timer_clock = tx_time_get();

        /* Compute the current milliseconds.  */
        *current_ms = (current_timer_clock - tick_counter -> timer_clock) * (1000 / TX_TIMER_TICKS_PER_SECOND);
    }
    return result;
}
