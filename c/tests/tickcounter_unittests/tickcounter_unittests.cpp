// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#include <cstdlib>
#ifdef _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#include <stddef.h>

#include "testrunnerswitcher.h"
#include "tickcounter.h"
#include "micromock.h"

static MICROMOCK_GLOBAL_SEMAPHORE_HANDLE g_dllByDll;

BEGIN_TEST_SUITE(tickcounter_UnitTests)

TEST_SUITE_INITIALIZE(suite_init)
{
    INITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

TEST_SUITE_CLEANUP(suite_cleanup)
{
    DEINITIALIZE_MEMORY_DEBUG(g_dllByDll);
}

END_TEST_SUITE(tickcounter_UnitTests)
