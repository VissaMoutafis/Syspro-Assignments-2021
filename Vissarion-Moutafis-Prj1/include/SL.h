#pragma once

#include "Types.h"

typedef struct skip_list *SL;   // the header of the skip list struct
typedef struct skip_list_node *SLNode; // the nodes that a skip list is made of

// Basic sl functions

// Creator function of the skip list.
SL sl_create(Compare compare, ItemDestructor itemDestructor, int max_level, float leveled_nodes_percent);

// The insert function of the skip list. Insert entry into sl. If the entry is already in the sl 
// we decide whether to replace or not by setting the replace flag properly 
void sl_insert(SL sl, Pointer entry, bool replace, Pointer *old_entry);

// The deletion function of the skip list. Delete entry from sl. If the flag destroy_entry is set to true 
// then the entry is destroyed and old_entry is null, otherwise the old entry points to the deleted nodes entry. 
// If the delete is successful return true, else return false
bool sl_delete(SL sl, Pointer entry, bool destroy_entry, Pointer *old_entry);

// Search for an entry in the sl. If found return the entry. 
// Otherwise return NULL 
Pointer sl_search(SL sl, Pointer search_entry);

// Function to free all the memory of the Skip List. WARNING the sl will be pointing to trash memory after this action
void sl_destroy(SL sl);

// Function to set a new destructor for the sl. The previous one is saved in prev_destructor pointer
void sl_set_destructor(SL sl, ItemDestructor new_destructor, ItemDestructor *prev_destructor);

// Funciton to print the entries
void sl_apply(SL sl, Apply visit);