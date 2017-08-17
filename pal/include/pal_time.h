// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PAL_TIME_H
#define PAL_TIME_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

    // Initialize NTP, and block until the time has been set
    int pal_time_init();
    void pal_time_deinit();

    // Return the time that the Azure IoT SDK should use for reporting times
    time_t pal_get_time(time_t* p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PAL_TIME_H */