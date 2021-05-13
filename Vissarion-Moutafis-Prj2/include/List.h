/**
*	Syspro Project 2
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#pragma once 

#include "Types.h"

typedef struct listnode* ListNode;
typedef struct list* List;

#define LIST_EOF ((ListNode)0)

//List methods

// Creates a list.
List list_create(Compare compare, ItemDestructor itemDestructor);

// Checks if the list is empty
bool list_empty(List list);

// Returns the length of the list
int list_len(List list);

// Get the first entry of the list
ListNode list_get_head(List list);

// Get the last entry of the list
ListNode list_get_tail(List list);

// Finds the node and returns it, otherwise returns NULL.
ListNode list_find(List list, Pointer entry);

// Inserts an entry in the list.
// If the last is false it is inserted at the beginning, else it is inserted at the end.
void list_insert(List list, Pointer entry, bool last);

// Delete a specific entry
bool list_delete(List list, Pointer entry, bool delete_entry, Pointer *old_entry);

// Sets a new itemDestructor for the list.
void list_set_destructor(List list, ItemDestructor itemDestructor);

// Return the destructor of the list
ItemDestructor list_get_destructor(List list);

// De-allocate the memory allocated for the list.
void list_destroy(List *list);

// List Node methods

// get the next node
ListNode list_get_next(List list, ListNode node);

ListNode list_get_prev(List list, ListNode node);

// Getter for the node entry
Pointer list_node_get_entry(List list, ListNode node);

// Set a new entry for a specific node.
void list_node_set_entry(List list, ListNode node, Pointer new_entry, Pointer *old_entry);


// Additional methods
// insert sorted based on given compare
// void list_insert_sorted(List list, Pointer entry, Compare compare);

// given a list and a comparison function we will return the first n elements
// List list_get_top_n(List list, Compare compare, int n);

// find the max node in a list based on user given compare
// Pointer list_find_max(List list, Compare compare);

// find the min non in a listed based on a user giver compare
// Pointer list_find_min(List list, Compare compare);

// simple print function
void list_print(List list, Visit visit);