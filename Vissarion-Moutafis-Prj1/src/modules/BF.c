#include "BF.h"

#include <stdio.h>

#define BITS_PER_UINT8_T 8

typedef unsigned char uint8_t;

struct bloom_filter {
    Hash_Func *func_list;       // The hash functions' array-list
    uint8_t *bit_string;        // The actual bit string for the bloom filter implementation
    u_int32_t size;             // size of the bloom filter
    u_int32_t hash_func_count;  // number of inserted hash functions
};

// Utility functions
static void write_bit(uint8_t *bit_string, size_t index) {
    // we must find the 8-bit word that the index we are talking about is placed
    size_t word_index = index / BITS_PER_UINT8_T;
    uint8_t *word = &bit_string[word_index];

    // now we will find the bit index in the word we chose
    size_t bit_index = index % BITS_PER_UINT8_T;
    uint8_t i = 1;
    *word |= (i << bit_index);  // set the bit in word[bit_index] equal to '1'
}

static bool read_bit(uint8_t *bit_string, size_t index) {
    size_t word_index = index / BITS_PER_UINT8_T;
    uint8_t *word = &bit_string[word_index];

    // now we will find the bit index in the word we chose
    size_t bit_index = index % BITS_PER_UINT8_T;

    // check if the bit at word[bit_index] is equal to '1'
    // and return true if it is and false if its not
    return (((*word & (0x1 << bit_index)) >> bit_index) & 0x1);
}

BF bf_create(size_t hash_func_count, size_t size) {
    BF bf = malloc(sizeof(*bf));

    assert(hash_func_count <= size);

    bf->func_list = calloc(hash_func_count, sizeof(Hash_Func));
    bf->hash_func_count = hash_func_count;
    bf->size = size;
    bf->bit_string = calloc((bf->size / BITS_PER_UINT8_T) + 1, sizeof(uint8_t));

    return bf;
}

void bf_destroy(BF bf) {
    free(bf->bit_string);
    free(bf->func_list);
    free(bf);
}


// Function to add and entry into the bloom filter
void bf_insert(BF bf, Pointer entry) {
    assert(bf != NULL);
    
    size_t total_bits = bf->size;

    for (int i = 0; i < bf->hash_func_count; i++) {
        // for every hash function estimate the hash value and the index that you should change to 1
        size_t index = hash_i((unsigned char *)entry, i) % total_bits;
        // write the proper bit
        write_bit(bf->bit_string, index);
    }
}

// Function that returns true whether the entry might be in the bloom filter and
// false otherwise
bool bf_contains(BF bf, Pointer entry) {
    assert(bf != NULL);

    size_t total_bits = bf->size;

    for (int i = 0; i < bf->hash_func_count; i++) {
        // find the index to check
        size_t index = hash_i((unsigned char *)entry, i) % total_bits;
        // if the bit in the specific index is equal to 1 continue, else return false
        if (!read_bit(bf->bit_string, index)) return false;
    }

    // if we reached here then all checks have succeeded, the entry is possible to have been inserted in the BF. Return true
    return true;
}



// static bool is_prime(size_t num) {
//     if (num == 1) return false;
//     if (num % 2 == 0 && num > 2) return false;
//     if (num % 3 == 0 && num > 3) return false;

//     for (size_t i = 5; i * i <= num; i += 6)
//         if (num % i == 0 || num % (i + 2) == 0) return false;

//     return true;
// }


///////////////////////////////////

// test main
int main(void) {
    BF bf = bf_create(15, 100);

    for (int i = 0; i < 100; i++) {
        if (!(i%7)){
            printf("Inserting %d.\n", i);
            char buf[100];
            memset(buf, 0, 100);
            sprintf(buf, "%d", i);
            bf_insert(bf, buf);
        }
    }

    for (int i = 0; i < 100; i++) {
        char buf[100];
        memset(buf, 0, 100);
        sprintf(buf, "%d", i);
        if (!bf_contains(bf, buf)) printf("%d, %f not in BF.\n", i, ((float)i/7.0));
    }
    bf_destroy(bf);
    return 0;
}