// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/xio.h"
#include "azure_c_shared_utility/tlsio_openssl.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>

int platform_init(void)
{
    return tlsio_openssl_init();
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_openssl_get_interface_description();
}

STRING_HANDLE platform_get_platform_info(void)
{
    STRING_HANDLE result;
    struct utsname nnn;

    if (uname(&nnn) == 0)
    {
        result = STRING_construct_sprintf("(%s; %s)", nnn.sysname, nnn.machine);
    }
    else
    {
        LogInfo("WARNING: failed to find machine info.");
        result = STRING_construct("Linux");
    }

    return result;
}

void platform_deinit(void)
{
	tlsio_openssl_deinit();
}
