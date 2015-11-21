// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stdint.h>
#include <time.h>
#include "tickcounter.h"

typedef struct TICK_COUNTER_INSTANCE_TAG
{
	clock_t last_clock_value;
	uint64_t current_ms;
} TICK_COUNTER_INSTANCE;

TICK_COUNTER_HANDLE tickcounter_create(void)
{
	TICK_COUNTER_INSTANCE* result = (TICK_COUNTER_INSTANCE*)malloc(sizeof(TICK_COUNTER_INSTANCE));
	if (result != NULL)
	{
		result->last_clock_value = clock();
		result->current_ms = result->last_clock_value * 1000 / CLOCKS_PER_SEC;
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

int tickcounter_get_current_ms(TICK_COUNTER_HANDLE tick_counter, uint64_t* current_ms)
{
	int result;

	if (tick_counter == NULL)
	{
		result = __LINE__;
	}
	else
	{
		TICK_COUNTER_INSTANCE* tick_counter_instance = (TICK_COUNTER_INSTANCE*)tick_counter;

		clock_t clock_value = clock();

		tick_counter_instance->current_ms += (clock_value - tick_counter_instance->last_clock_value) * 1000 / CLOCKS_PER_SEC;
		tick_counter_instance->last_clock_value = clock_value;
		*current_ms = tick_counter_instance->current_ms;

		result = 0;
	}

	return result;
}
