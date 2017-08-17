// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PAL_TIME_H
#define PAL_TIME_H

#include <time.h>

#ifdef __cplusplus
#include <cstdbool>
extern "C" {
#include <stdbool.h>
#endif /* __cplusplus */

    // Initialize NTP, and optionally block until the time has been set
    int pal_time_init(bool blocking);
    void pal_time_deinit();

    // Return true if system time is at least approximately good
    bool pal_time_is_reliable();

    // Return the time that the Azure IoT SDK should use for reporting times and checking cert validity
    time_t pal_get_time(time_t* p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PAL_TIME_H */