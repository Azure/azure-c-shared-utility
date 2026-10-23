// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>

#include "azure_c_shared_utility/platform.h"
/* logger.h first: with NO_LOGGING, xlogging.h must be the one that defines the log macros. */
#include "c_logging/logger.h"
#include "azure_c_shared_utility/xlogging.h"

/* Logging with c-logging aborts while it is not initialized; each check below fails by aborting
   if platform_init did not initialize it. */

static int platform_init_makes_logging_usable(void)
{
    int result = platform_init();
    if (result == 0)
    {
        LogInfo("platform_int: LogInfo after platform_init");
        LogError("platform_int: LogError after platform_init");
        platform_deinit();
    }
    return result;
}

static int application_logger_init_is_kept_after_platform_deinit(void)
{
    /* Applications that call logger_init themselves (needed before platform_init initialized it) keep working. */
    int result;
    if (logger_init() != 0)
    {
        result = 1;
    }
    else
    {
        result = platform_init();
        if (result == 0)
        {
            platform_deinit();
            LogError("platform_int: LogError after platform_deinit, application logger_init outstanding");
        }
        logger_deinit();
    }
    return result;
}

static int extra_platform_deinit_keeps_application_logger(void)
{
    /* Init and deinit twice, ignoring the second init's result (it fails on Linux: OpenSSL init is not
       re-entrant). Deinit must not release more logger references than successful inits took. */
    int result;
    if (logger_init() != 0)
    {
        result = 1;
    }
    else
    {
        result = platform_init();
        if (result == 0)
        {
            (void)platform_init();
            platform_deinit();
            platform_deinit();
            LogError("platform_int: application log after extra platform_deinit");
        }
        logger_deinit();
    }
    return result;
}

int main(void)
{
    int failed = 0;

    if (platform_init_makes_logging_usable() != 0)
    {
        (void)printf("FAILED: platform_init_makes_logging_usable\r\n");
        failed++;
    }

    /* Runs after a full platform_deinit, so it also covers initializing again. */
    if (platform_init_makes_logging_usable() != 0)
    {
        (void)printf("FAILED: platform_init_makes_logging_usable (second time)\r\n");
        failed++;
    }

    if (application_logger_init_is_kept_after_platform_deinit() != 0)
    {
        (void)printf("FAILED: application_logger_init_is_kept_after_platform_deinit\r\n");
        failed++;
    }

    if (extra_platform_deinit_keeps_application_logger() != 0)
    {
        (void)printf("FAILED: extra_platform_deinit_keeps_application_logger\r\n");
        failed++;
    }

    (void)printf("platform_int: %d failed\r\n", failed);
    return failed;
}
