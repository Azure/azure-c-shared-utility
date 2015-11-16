// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "platform.h"
#include "tlsio_openssl.h"

int platform_init(void)
{
	tlsio_openssl_init();

	return 0;
}

void platform_deinit(void)
{
	tlsio_openssl_deinit();
}
