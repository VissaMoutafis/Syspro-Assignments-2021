/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/

#pragma once

#include "Types.h"

typedef struct circular_buffer *CB;

// default circular size is 
#define DEF_CB_SIZE 10

// create a circular buffer of size <size> with items that can be destroyed with destroy
CB cb_create(int size, ItemDestructor destroy);

// Add an item in the circular buffer and get a pointer to it. On success we return true. If the buffer is full we will return false
bool cb_add(CB cb, Pointer item);

// Get an item from the cb and return a void * to it. WARNING this memory is not malloc'd so do not delete it.
Pointer cb_get(CB cb); 

// destroy the circular buffer 
void cb_destroy(CB cb);

bool cb_is_full(CB cb);

bool cb_is_empty(CB cb);