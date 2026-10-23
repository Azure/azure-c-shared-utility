// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/tlsio_mbedtls.h"

/* c-logging aborts on any log line until logger_init is called; platform_init owns that reference.
   Unit tests predefine PLATFORM_LOGGER_INIT/PLATFORM_LOGGER_DEINIT to mock them. */
#ifndef PLATFORM_LOGGER_INIT
#if !defined(NO_LOGGING) && !defined(MINIMAL_LOGERROR) && !defined(ESP8266_RTOS)
#include "c_logging/logger.h"
#define PLATFORM_LOGGER_INIT() logger_init()
#define PLATFORM_LOGGER_DEINIT() logger_deinit()
#else
#define PLATFORM_LOGGER_INIT() 0
#define PLATFORM_LOGGER_DEINIT() ((void)0)
#endif
#endif

/* Successful platform_init calls not yet matched by platform_deinit. platform_deinit releases the logger
   reference only for those, so extra calls (e.g. after a failed platform_init) keep the caller's logger. */
static unsigned int platform_logger_refs = 0;

int platform_init(void)
{
    int result = PLATFORM_LOGGER_INIT();
    if (result == 0)
    {
        platform_logger_refs++;
    }
    return result;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_mbedtls_get_interface_description();
}

STRING_HANDLE platform_get_platform_info(PLATFORM_INFO_OPTION options)
{
    // No applicable options, so ignoring parameter
    (void)options;

    // Expected format: "(<runtime name>; <operating system name>; <platform>)"

    return STRING_construct("(native; tizenrt; undefined)");
}

void platform_deinit(void)
{
    if (platform_logger_refs > 0)
    {
        platform_logger_refs--;
        PLATFORM_LOGGER_DEINIT();
    }
}
