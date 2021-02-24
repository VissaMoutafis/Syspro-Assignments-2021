#pragma once

#include "Types.h"

#define HASH_NUM 16 // this is the number of hash functions

typedef struct bloom_filter *BF;

// Bloom Filter Basic Methods

// Create the Bloom Filter.
BF bf_create(Hash_Func *func_list, size_t hash_func_count, size_t entry_size) {
