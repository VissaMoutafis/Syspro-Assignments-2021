/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#pragma once

#include "Types.h"
#include "Config.h"

typedef struct bloom_filter *BF;

// Bloom Filter Basic Methods

// Create the Bloom Filter.
BF bf_create(size_t hash_func_count, size_t entry_size);

// Insert the entry to the bloom filter
void bf_insert(BF bf, Pointer entry);

// Check if the entry exists
bool bf_contains(BF bf, Pointer entry);

// Free the memory of the bloom filter
void bf_destroy(BF bf);


// Extensions for syspro prj2

#define BF_START_TAG "<bf>"
#define BF_END_TAG "</bf>"

// create a Bloom filter from a string buffer with serial data, 
// based on a separator given by the user. In error return NULL
BF bf_create_from_buffer(char *buffer, int len, char sep);

// write data to a buffer from the given BF, separated by a user provided sep
void bf_to_buffer(BF bf, char **dest_buf, int *buf_len, char sep);

// write data to a file (given file descriptor) from the given BF, separated by a user provided sep
void bf_to_file(BF bf, int fd, char sep);


// perfom union of bf and to_unite and save result in the <bf> BloomFilter
void bf_union(BF bf, BF to_unite);