#include "BF.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define BITS_PER_UINT8_T 8

typedef unsigned char uint8_t;

typedef struct bf_hash {
    Hash_Func func;
    struct bf_hash *next;
} * BF_Hash;

struct bloom_filter {
    BF_Hash func_list;
    uint8_t *bit_string;
    size_t size;
    size_t entry_size;
    size_t hash_func_count;
};

void bf_add_hash_func(BF bf, Hash_Func func) {
    assert(bf != NULL);
    if (func) {
        BF_Hash h = bf->func_list;
        while (h && h->next != NULL) h = h->next;

        // we get to the end of the hash function list
        // now we have to add the new function there

        BF_Hash a = malloc(sizeof(struct bf_hash));
        a->next = NULL;
        a->func = func;
        bf->hash_func_count++;
        if (h)
            h->next = a;
        else
            bf->func_list = a;
    }
}

static size_t set_size(size_t entry_size);

BF bf_create(Hash_Func *func_list, size_t hash_func_count, size_t entry_size) {
    BF bf = malloc(sizeof(*bf));

    assert(entry_size > 0);

    bf->func_list = NULL;
    bf->hash_func_count = 0;
    // initialize the hash funtion list
    if (hash_func_count > 0)
        for (int i = 0; i < hash_func_count; ++i)
            bf_add_hash_func(bf, func_list[i]);

    bf->size = set_size(entry_size);
    bf->entry_size = 0;
    bf->bit_string = calloc((bf->size / BITS_PER_UINT8_T) + 1, sizeof(uint8_t));

    return bf;
}

void bf_destroy(BF bf) {
    if (bf != NULL) {
        BF_Hash i, h = bf->func_list;
        while (h) {
            i = h->next;
            free(h);
            h = i;
        }
        free(bf->bit_string);
        free(bf);
    }
}

static void write_bit(uint8_t *bit_string, size_t index);

// Function to add and entry into the bloom filter
void bf_insert(BF bf, Pointer entry) {
    assert(bf != NULL);

    size_t total_bits = bf->size;
    BF_Hash hash = bf->func_list;

    while (hash) {
        size_t index = hash->func(entry) % total_bits;
        write_bit(bf->bit_string, index);
        hash = hash->next;
    }
}
static bool read_bit(uint8_t *bit_string, size_t index);

// Function that returns true whether the entry might be in the bloom filter and
// false otherwise
bool bf_contains(BF bf, Pointer entry) {
    assert(bf != NULL);

    size_t total_bits = bf->size;
    BF_Hash hash = bf->func_list;

    while (hash) {
        size_t index = hash->func(entry) % total_bits;
        if (!read_bit(bf->bit_string, index)) return false;
        hash = hash->next;
    }

    return true;
}

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

static bool is_prime(size_t num) {
    if (num == 1) return false;
    if (num % 2 == 0 && num > 2) return false;
    if (num % 3 == 0 && num > 3) return false;

    for (size_t i = 5; i * i <= num; i += 6)
        if (num % i == 0 || num % (i + 2) == 0) return false;

    return true;
}

static size_t set_size(size_t entry_size) {
    // we must set the bloom filter size at least as large as 3*entry_size and
    // it must be a prime number
    size_t bf_size = 3 * entry_size;

    while (!is_prime(bf_size)) bf_size++;
    return bf_size;
}

///////////////////////////////////

// Pointer create_int(int a)
// {
//     int *pa = malloc(sizeof(int));
//     *pa = a;
//     Pointer p = pa;
//     return p;
// }

// size_t h1(Pointer a)
// {
//     return (*(int*)a);
// }
// size_t h2(Pointer a)
// {
//     return 5*(*(int*)a) + 4;
// }

// int main(void)
// {
//     Hash_Func h[2] = {&h1, &h2};
//     BF bf = bf_create(h, 2, 3);

//     bf_insert(bf, create_int(19));
//     bf_insert(bf, create_int(132));
//     bf_insert(bf, create_int(25));

//     bf_contains(bf, create_int(132)) == true ? printf("132 Yes\n") :
//     printf("132 No\n"); bf_contains(bf, create_int(19)) == true ? printf("19
//     Yes\n") : printf("19 No\n"); bf_contains(bf, create_int(133)) == true ?
//     printf("133 Yes\n") : printf("133 No\n"); bf_contains(bf, create_int(22))
//     == true ? printf("22 Yes (Wrong positive)\n") : printf("22 No\n");
//     bf_contains(bf, create_int(25)) == true ? printf("25 Yes\n") : printf("25
//     No\n");

// }