// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef GBALLOC_H
#define GBALLOC_H

#include "azure_c_shared_utility/umock_c_prod.h"

#ifdef __cplusplus
#include <cstddef>
#include <cstdlib>
extern "C"
{
#else
#include <stddef.h>
#include <stdlib.h>
#endif

/* all translation units that need memory measurement need to have GB_MEASURE_MEMORY_FOR_THIS defined */
/* GB_DEBUG_ALLOC is the switch that turns the measurement on/off, so that it is not on always */
#if defined(GB_DEBUG_ALLOC)

DLLEXPORT MOCKABLE_FUNCTION(, int, gballoc_init);
DLLEXPORT MOCKABLE_FUNCTION(, void, gballoc_deinit);
DLLEXPORT MOCKABLE_FUNCTION(, void*, gballoc_malloc, size_t, size);
DLLEXPORT MOCKABLE_FUNCTION(, void*, gballoc_calloc, size_t, nmemb, size_t, size);
DLLEXPORT MOCKABLE_FUNCTION(, void*, gballoc_realloc, void*, ptr, size_t, size);
DLLEXPORT MOCKABLE_FUNCTION(, void, gballoc_free, void*, ptr);

DLLEXPORT MOCKABLE_FUNCTION(, size_t, gballoc_getMaximumMemoryUsed);
DLLEXPORT MOCKABLE_FUNCTION(, size_t, gballoc_getCurrentMemoryUsed);
DLLEXPORT MOCKABLE_FUNCTION(, size_t, gballoc_getAllocationCount);
DLLEXPORT MOCKABLE_FUNCTION(, void, gballoc_resetMetrics);

/* if GB_MEASURE_MEMORY_FOR_THIS is defined then we want to redirect memory allocation functions to gballoc_xxx functions */
#ifdef GB_MEASURE_MEMORY_FOR_THIS
/* Unfortunately this is still needed here for things to still compile when using _CRTDBG_MAP_ALLOC.
That is because there is a rogue component (most likely CppUnitTest) including crtdbg. */
#if defined(_CRTDBG_MAP_ALLOC) && defined(_DEBUG)
#undef _malloc_dbg
#undef _calloc_dbg
#undef _realloc_dbg
#undef _free_dbg
#define _malloc_dbg(size, ...) gballoc_malloc(size)
#define _calloc_dbg(nmemb, size, ...) gballoc_calloc(nmemb, size)
#define _realloc_dbg(ptr, size, ...) gballoc_realloc(ptr, size)
#define _free_dbg(ptr, ...) gballoc_free(ptr)
#else
#define malloc gballoc_malloc
#define calloc gballoc_calloc
#define realloc gballoc_realloc
#define free gballoc_free
#endif
#endif

#else /* GB_DEBUG_ALLOC */

#define gballoc_init() 0
#define gballoc_deinit() ((void)0)

#define gballoc_getMaximumMemoryUsed() SIZE_MAX
#define gballoc_getCurrentMemoryUsed() SIZE_MAX
#define gballoc_getAllocationCount() SIZE_MAX
#define gballoc_resetMetrics() ((void)0)

#endif /* GB_DEBUG_ALLOC */

#ifdef __cplusplus
}
#endif

#endif /* GBALLOC_H */
