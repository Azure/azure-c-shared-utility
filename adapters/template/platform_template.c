// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

/* This is a template file used for porting */

/* Please go through all the TODO sections below and implement the needed code */

#include "azure_c_shared_utility/platform.h"

int platform_init(void)
{
	/* TODO: Insert here any platform specific one time initialization code like:
	- starting TCP stack
	- starting timer
	- etc.
	*/
	
	return 0;
}

const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
	/* TODO: Insert here a call to the tlsio adapter that is available on your platform.
	return tlsio_mytls_get_interface_description(); */
}

STRING_HANDLE platform_get_platform_info(void)
{
    /* TODO: Insert here a call interrogate the platform for its OS Name and Version,
       also a call to get the board's architecture. return a string in the following form:
       (OS Name OS Version; ARCH) - eg (Ubuntu 16.04.02 LTS; armv71) */
}

void platform_deinit(void)
{
	/* TODO: Insert here any platform specific one time de-initialization code like.
	This has to undo what has been done in platform_init. */
}
