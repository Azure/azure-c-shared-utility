singlylinkedlist requirements
================
 
##Overview

SinglyLinkedList is module that provides the functionality of a singly linked list, allowing its user to add, remove and iterate the list elements.

##Exposed API

```c
typedef struct SINGLYLINKEDLIST_INSTANCE_TAG* SINGLYLINKEDLIST_HANDLE;
typedef struct LIST_ITEM_INSTANCE_TAG* LIST_ITEM_HANDLE;
typedef bool (*LIST_MATCH_FUNCTION)(LIST_ITEM_HANDLE list_item, const void* match_context);

extern SINGLYLINKEDLIST_HANDLE singlylinkedlist_create(void);
extern void singlylinkedlist_destroy(SINGLYLINKEDLIST_HANDLE list);
extern LIST_ITEM_HANDLE singlylinkedlist_add(SINGLYLINKEDLIST_HANDLE list, const void* item);
extern int singlylinkedlist_remove(SINGLYLINKEDLIST_HANDLE list, LIST_ITEM_HANDLE item_handle);
extern LIST_ITEM_HANDLE singlylinkedlist_get_head_item(SINGLYLINKEDLIST_HANDLE list);
extern LIST_ITEM_HANDLE singlylinkedlist_get_next_item(LIST_ITEM_HANDLE item_handle);
extern LIST_ITEM_HANDLE singlylinkedlist_find(SINGLYLINKEDLIST_HANDLE list, LIST_MATCH_FUNCTION match_function, const void* match_context);
extern int singlylinkedlist_remove_matching_item(SINGLYLINKEDLIST_HANDLE list, LIST_MATCH_FUNCTION match_function, const void* match_context);

extern const void* singlylinkedlist_item_get_value(LIST_ITEM_HANDLE item_handle);
```

###singlylinkedlist_create
```c
extern SINGLYLINKEDLIST_HANDLE singlylinkedlist_create(void);
```

**SRS_LIST_01_001: [**singlylinkedlist_create shall create a new list and return a non-NULL handle on success.**]**
**SRS_LIST_01_002: [**If any error occurs during the list creation, singlylinkedlist_create shall return NULL.**]**
 
###singlylinkedlist_destroy
```c
extern void singlylinkedlist_destroy(SINGLYLINKEDLIST_HANDLE list);
```
**SRS_LIST_01_003: [**singlylinkedlist_destroy shall free all resources associated with the list identified by the handle argument.**]**
**SRS_LIST_01_004: [**If the list argument is NULL, no freeing of resources shall occur.**]**
 
###singlylinkedlist_add
```c
extern int singlylinkedlist_add(SINGLYLINKEDLIST_HANDLE list, const void* item);
```

**SRS_LIST_01_005: [**singlylinkedlist_add shall add one item to the tail of the list and on success it shall return a handle to the added item.**]**
**SRS_LIST_01_006: [**If any of the arguments is NULL, singlylinkedlist_add shall not add the item to the list and return NULL.**]**
**SRS_LIST_01_007: [**If allocating the new list node fails, singlylinkedlist_add shall return NULL.**]**
 
###singlylinkedlist_get_head_item
```c
extern const void* singlylinkedlist_get_head_item(SINGLYLINKEDLIST_HANDLE list);
```
**SRS_LIST_01_008: [**singlylinkedlist_get_head_item shall return the head of the list.**]**
**SRS_LIST_01_009: [**If the list argument is NULL, singlylinkedlist_get_head_item shall return NULL.**]**
**SRS_LIST_01_010: [**If the list is empty, list_get_head_item_shall_return NULL.**]**
 
###singlylinkedlist_get_next_item
```c
extern LIST_ITEM_HANDLE singlylinkedlist_get_next_item(LIST_ITEM_HANDLE item_handle);
```

**SRS_LIST_01_018: [**singlylinkedlist_get_next_item shall return the next item in the list following the item item_handle.**]**
**SRS_LIST_01_019: [**If item_handle is NULL then singlylinkedlist_get_next_item shall return NULL.**]**
**SRS_LIST_01_022: [**If no more items exist in the list after the item_handle item, singlylinkedlist_get_next_item shall return NULL.**]** 

###singlylinkedlist_find
```c
extern const void* singlylinkedlist_find(SINGLYLINKEDLIST_HANDLE list, const void* match_context, LIST_MATCH_FUNCTION match_function);
```

**SRS_LIST_01_011: [**singlylinkedlist_find shall iterate through all items in a list and return the first one that satisfies a certain match function.**]**
**SRS_LIST_01_012: [**If the list or the match_function argument is NULL, singlylinkedlist_find shall return NULL.**]**
**SRS_LIST_01_014: [**list find shall determine whether an item satisfies the match criteria by invoking the match function for each item in the list until a matching item is found.**]**
**SRS_LIST_01_013: [**The match_function shall get as arguments the list item being attempted to be matched and the match_context as is.**]**
**SRS_LIST_01_016: [**If the match function returns false, singlylinkedlist_find shall consider that item as not matching.**]**
**SRS_LIST_01_017: [**If the match function returns true, singlylinkedlist_find shall consider that item as matching.**]**
**SRS_LIST_01_015: [**If the list is empty, singlylinkedlist_find shall return NULL.**]**
 
###singlylinkedlist_remove
```c
extern int singlylinkedlist_remove(SINGLYLINKEDLIST_HANDLE list, LIST_ITEM_HANDLE item_handle);
```

**SRS_LIST_01_023: [**singlylinkedlist_remove shall remove a list item from the list and on success it shall return 0.**]**
**SRS_LIST_01_024: [**If any of the arguments list or item_handle is NULL, singlylinkedlist_remove shall fail and return a non-zero value.**]**
**SRS_LIST_01_025: [**If the item item_handle is not found in the list, then singlylinkedlist_remove shall fail and return a non-zero value.**]**
 
###singlylinkedlist_item_get_value
```c
extern const void* singlylinkedlist_item_get_value(LIST_ITEM_HANDLE item_handle);
```

**SRS_LIST_01_020: [**singlylinkedlist_item_get_value shall return the value associated with the list item identified by the item_handle argument.**]**
**SRS_LIST_01_021: [**If item_handle is NULL, singlylinkedlist_item_get_value shall return NULL.**]**
