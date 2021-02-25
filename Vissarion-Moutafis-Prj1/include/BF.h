#pragma once

#include "Types.h"
 
typedef struct bloom_filter *BF;

// hash function needed
unsigned long hash_i(unsigned char *str, unsigned int i);

// Bloom Filter Basic Methods

// Create the Bloom Filter.
BF bf_create(size_t hash_func_count, size_t entry_size);

// Insert the entry to the bloom filter
void bf_insert(BF bf, Pointer entry);

// Check if the entry exists
bool bf_contains(BF bf, Pointer entry);


// Free the memory of the bloom filter
void bf_destroy(BF bf);