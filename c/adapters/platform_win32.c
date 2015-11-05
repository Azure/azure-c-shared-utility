// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include "platform.h"
#include "winsock2.h"

int platform_init(void)
{
	int result;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		result = __LINE__;
	}
	else
	{
		result = 0;
	}

	return result;
}

void platform_deinit(void)
{
	(void)WSACleanup();
}
