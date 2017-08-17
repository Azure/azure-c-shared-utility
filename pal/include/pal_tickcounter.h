// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PAL_TICKCOUNTER_H
#define PAL_TICKCOUNTER_H

#ifdef __cplusplus
extern "C" {
#include <cstdint>
#include <cstdbool>
#else
#include <stdint.h>
#include <stdbool.h>
#endif /* __cplusplus */

    // Values are typedef'd as uint32_t instead of uint_fast32_t so that native
    // 32 bit tickcounters can always be used directly, even if the app is
    // compiled for 64 bit integers.
    typedef uint32_t tickcounter_count_ms_t;
    typedef uint32_t tickcounter_interval_ms_t;

    // Start the 32-bit free-running counter for pal_tickcounter_get_current_ms
    int pal_tickcounter_init();
    void pal_tickcounter_deinit();

    // Returns true if pal_tickcounter_init succeeded in starting the counter
    bool pal_tickcounter_started_ok();

    // Return the current value of the free-running counter. Returns 0 if the counter could not be started.
    tickcounter_count_ms_t pal_tickcounter_get_count_ms();

    // Return the interval from previous_count until now
    // pal_tickcounter_get_interval_ms is part of the Azure IoT SDK and is not needed for new PAL implementations
    tickcounter_interval_ms_t pal_tickcounter_get_interval_ms(tickcounter_count_ms_t previous_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PAL_TICKCOUNTER_H */