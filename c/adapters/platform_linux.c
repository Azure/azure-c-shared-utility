// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "platform.h"
#include "xio.h"
#include "tlsio_openssl.h"

int platform_init(void)
{
	tlsio_openssl_init();

	return 0;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_openssl_get_interface_description();
}

void platform_deinit(void)
{
	tlsio_openssl_deinit();
}
