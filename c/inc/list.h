// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LIST_H
#define LIST_H

#ifdef __cplusplus
extern "C" {
#include "cstdbool"
#else
#include "stdbool.h"
#endif /* __cplusplus */

	typedef void* LIST_HANDLE;
	typedef void* LIST_ITEM_HANDLE;
	typedef bool (*LIST_MATCH_FUNCTION)(LIST_ITEM_HANDLE list_item, const void* match_context);

	extern LIST_HANDLE list_create(void);
	extern void list_destroy(LIST_HANDLE list);
	extern LIST_ITEM_HANDLE list_add(LIST_HANDLE list, const void* item);
	extern int list_remove(LIST_HANDLE list, LIST_ITEM_HANDLE item_handle);
	extern LIST_ITEM_HANDLE list_get_head_item(LIST_HANDLE list);
	extern LIST_ITEM_HANDLE list_get_next_item(LIST_ITEM_HANDLE item_handle);
	extern const void* list_item_get_value(LIST_ITEM_HANDLE item_handle);
	extern LIST_ITEM_HANDLE list_find(LIST_HANDLE list, LIST_MATCH_FUNCTION match_function, const void* match_context);
	extern int list_remove_matching_item(LIST_HANDLE list, LIST_MATCH_FUNCTION match_function, const void* match_context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LIST_H */
