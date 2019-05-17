// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define CONSTBUFFER_Create real_CONSTBUFFER_Create
#define CONSTBUFFER_CreateFromBuffer real_CONSTBUFFER_CreateFromBuffer
#define CONSTBUFFER_CreateWithMoveMemory real_CONSTBUFFER_CreateWithMoveMemory
#define CONSTBUFFER_CreateWithCustomFree real_CONSTBUFFER_CreateWithCustomFree
#define CONSTBUFFER_CreateFromOffsetAndSize real_CONSTBUFFER_CreateFromOffsetAndSize
#define CONSTBUFFER_IncRef real_CONSTBUFFER_IncRef
#define CONSTBUFFER_DecRef real_CONSTBUFFER_DecRef
#define CONSTBUFFER_GetContent real_CONSTBUFFER_GetContent
#define CONSTBUFFER_HANDLE_contain_same real_CONSTBUFFER_HANDLE_contain_same


#define GBALLOC_H

#include "constbuffer.c"
