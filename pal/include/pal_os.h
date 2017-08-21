// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PAL_OS_H
#define PAL_OS_H

#include <time.h>

#ifdef __cplusplus
#include <cstdbool>
extern "C" {
#include <stdbool.h>
#endif /* __cplusplus */

    // Tickcounter vlues are typedef'd as uint32_t instead of uint_fast32_t so that native
    // 32 bit tickcounters can always be used directly, even if the app is
    // compiled for 64 bit integers.
    typedef uint32_t tickcounter_count_ms_t;
    typedef uint32_t tickcounter_interval_ms_t;

    // Return the current value of the free-running counter.
    tickcounter_count_ms_t pal_tickcounter_get_count_ms();

    // Return the interval from previous_count until now. Recommended to ensure correct underflow
    // behavior during interval calculation. This is part of the Azure IoT SDK and need not be
    // written for new PAL implementations
    tickcounter_interval_ms_t pal_tickcounter_get_interval_ms(tickcounter_count_ms_t previous_count);

    // Return the time that the Azure IoT SDK should use for reporting times and checking cert validity
    time_t pal_get_time(time_t* p);

    // Sleep the current thread for at least the given number of milliseconds
    void pal_thread_sleep(unsigned int milliseconds);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PAL_OS_H */