// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <cstdlib>

#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include <cstdio>
#include <cctype>
#include "mbed.h"
#include "tickcounter.h"
#include <limits.h>

class TICK_COUNTER_INSTANCE_TAG
{
public:
    mbed::Timer timer_object;
    uint64_t current_ms;
    uint64_t overflow_ms;
};

TICK_COUNTER_HANDLE tickcounter_create(void)
{
    TICK_COUNTER_INSTANCE_TAG* result;
    result = new TICK_COUNTER_INSTANCE_TAG();
    result->timer_object.start();
    result->current_ms = result->timer_object.read_ms();
    result->overflow_ms = 0;
    return result;
}

void tickcounter_destroy(TICK_COUNTER_HANDLE tick_counter)
{
    if (tick_counter != NULL)
    {
        TICK_COUNTER_INSTANCE_TAG* tick_counter_instance = (TICK_COUNTER_INSTANCE_TAG*)tick_counter;
        tick_counter_instance->timer_object.stop();
        delete tick_counter;
    }
}

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, uint64_t* current_ms)
{
    int result;
    if (tick_counter == NULL || current_ms == NULL)
    {
        result = __LINE__;
    }
    else
    {
        TICK_COUNTER_INSTANCE_TAG* tick_counter_instance = (TICK_COUNTER_INSTANCE_TAG*)tick_counter;
        int ms_value = tick_counter_instance->timer_object.read_ms();
        if (ms_value < tick_counter_instance->current_ms)
        {
            tick_counter_instance->overflow_ms += INT_MAX;
        }

        tick_counter_instance->current_ms = tick_counter_instance->overflow_ms + ms_value;
        *current_ms = tick_counter_instance->current_ms;
        result = 0;
    }
    return result;
}
