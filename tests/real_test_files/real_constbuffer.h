// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef REAL_CONSTBUFFER_H
#define REAL_CONSTBUFFER_H

#define CONSTBUFFER                    real_CONSTBUFFER
#define CONSTBUFFER_TAG                real_CONSTBUFFER_TAG
#define CONSTBUFFER_Create             real_CONSTBUFFER_Create
#define CONSTBUFFER_Clone              real_CONSTBUFFER_Clone
#define CONSTBUFFER_CreateFromBuffer   real_CONSTBUFFER_CreateFromBuffer
#define CONSTBUFFER_GetContent         real_CONSTBUFFER_GetContent
#define CONSTBUFFER_Destroy            real_CONSTBUFFER_Destroy
#define CONSTBUFFER_CreateWithMoveMemory    real_CONSTBUFFER_CreateWithMoveMemory
#define CONSTBUFFER_CreateWithCustomFree    real_CONSTBUFFER_CreateWithCustomFree
#define CONSTBUFFER_CreateFromOffsetAndSize real_CONSTBUFFER_CreateFromOffsetAndSize
#define CONSTBUFFER_IncRef              real_CONSTBUFFER_IncRef
#define CONSTBUFFER_DecRef              real_CONSTBUFFER_DecRef
#define CONSTBUFFER_HANDLE_contain_same real_CONSTBUFFER_HANDLE_contain_same


#include "azure_c_shared_utility/constbuffer.h"

#ifndef COMPILING_REAL_CONSTBUFFER_C
#undef CONSTBUFFER_Create
#undef CONSTBUFFER_Clone
#undef CONSTBUFFER_CreateFromBuffer
#undef CONSTBUFFER_GetContent
#undef CONSTBUFFER_Destroy
#undef CONSTBUFFER_CreateWithMoveMemory
#undef CONSTBUFFER_CreateWithCustomFree
#undef CONSTBUFFER_CreateFromOffsetAndSize
#undef CONSTBUFFER_IncRef
#undef CONSTBUFFER_DecRef
#undef CONSTBUFFER_HANDLE_contain_same
#undef CONSTBUFFER_TAG
#undef CONSTBUFFER

#undef CONSTBUFFER_H

#else
#endif



#endif
