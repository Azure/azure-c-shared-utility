// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef PAL_SAMPLE_RUNNER_H
#define PAL_SAMPLE_RUNNER_H

#include <time.h>

#ifdef __cplusplus
#include <cstdbool>
extern "C" {
#include <stdbool.h>
#endif /* __cplusplus */

    typedef void(*azure_iot_sample_program_t)();

    // Perform any necessary device-dependent initialization, run the sample, then 
    // de-initialize and exit.
    azure_iot_sample_program_t azure_iot_sample_program;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PAL_SAMPLE_RUNNER_H */