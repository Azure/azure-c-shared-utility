// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define CONSTBUFFER_Create real_CONSTBUFFER_Create
#define CONSTBUFFER_CreateFromBuffer real_CONSTBUFFER_CreateFromBuffer
#define CONSTBUFFER_CreateWithMoveMemory real_CONSTBUFFER_CreateWithMoveMemory
#define CONSTBUFFER_CreateWithCustomFree real_CONSTBUFFER_CreateWithCustomFree
#define CONSTBUFFER_Clone real_CONSTBUFFER_Clone
#define CONSTBUFFER_GetContent real_CONSTBUFFER_GetContent
#define CONSTBUFFER_Destroy real_CONSTBUFFER_Destroy

#define GBALLOC_H

#include "constbuffer.c"
