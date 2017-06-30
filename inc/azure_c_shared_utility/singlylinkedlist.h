// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef SINGLYLINKEDLIST_H
#define SINGLYLINKEDLIST_H

#ifdef __cplusplus
extern "C" {
#include <cstdbool>
#else
#include "stdbool.h"
#endif /* __cplusplus */

#include "azure_c_shared_utility/umock_c_prod.h"

typedef struct SINGLYLINKEDLIST_INSTANCE_TAG* SINGLYLINKEDLIST_HANDLE;
typedef struct LIST_ITEM_INSTANCE_TAG* LIST_ITEM_HANDLE;
typedef bool (*LIST_MATCH_FUNCTION)(LIST_ITEM_HANDLE list_item, const void* match_context);
typedef void (*LIST_CONDITION_FUNCTION)(const void* item, const void* match_context, bool* remove_item, bool* continue_processing);
typedef void (*LIST_ACTION_ACTION)(const void* item, const void* action_context, bool* continue_processing);

MOCKABLE_FUNCTION(, SINGLYLINKEDLIST_HANDLE, singlylinkedlist_create);
MOCKABLE_FUNCTION(, void, singlylinkedlist_destroy, SINGLYLINKEDLIST_HANDLE, list);
MOCKABLE_FUNCTION(, LIST_ITEM_HANDLE, singlylinkedlist_add, SINGLYLINKEDLIST_HANDLE, list, const void*, item);
MOCKABLE_FUNCTION(, int, singlylinkedlist_remove, SINGLYLINKEDLIST_HANDLE, list, LIST_ITEM_HANDLE, item_handle);
MOCKABLE_FUNCTION(, LIST_ITEM_HANDLE, singlylinkedlist_get_head_item, SINGLYLINKEDLIST_HANDLE, list);
MOCKABLE_FUNCTION(, LIST_ITEM_HANDLE, singlylinkedlist_get_next_item, LIST_ITEM_HANDLE, item_handle);
MOCKABLE_FUNCTION(, LIST_ITEM_HANDLE, singlylinkedlist_find, SINGLYLINKEDLIST_HANDLE, list, LIST_MATCH_FUNCTION, match_function, const void*, match_context);
MOCKABLE_FUNCTION(, const void*, singlylinkedlist_item_get_value, LIST_ITEM_HANDLE, item_handle);
MOCKABLE_FUNCTION(, int, singlylinkedlist_remove_if, SINGLYLINKEDLIST_HANDLE, list, LIST_CONDITION_FUNCTION, condition_function, const void*, match_context);
MOCKABLE_FUNCTION(, int, singlylinkedlist_foreach, SINGLYLINKEDLIST_HANDLE, list, LIST_ACTION_ACTION, action_function, const void*, action_context);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SINGLYLINKEDLIST_H */
