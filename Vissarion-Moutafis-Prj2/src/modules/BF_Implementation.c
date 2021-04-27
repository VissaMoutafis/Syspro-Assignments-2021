/*
** BF Implementation 
** Implemented by Vissarion Moutafis sdi1800119
*/

#include "BF.h"

#define BITS_PER_UINT8_T 8

typedef unsigned char uint8_t;

struct bloom_filter {
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

    bf->hash_func_count = hash_func_count;
    bf->size = size;
    bf->bit_string = calloc(bf->size, sizeof(uint8_t));

    return bf;
}

void bf_destroy(BF bf) {
    free(bf->bit_string);
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

// Extensions for syspro prj2

// create a Bloom filter from a string buffer with serial data, 
// based on a separator given by the user.
BF bf_create_from_buffer(char *buffer, int len, char sep) {
    // format <bf><hash_func_count><sep><size><sep><bit_string></bf>
    BF bf = calloc(1, sizeof(*bf));
    // read the buffer first
    u_int32_t *attributes = calloc(2, sizeof(*attributes));

    // check opening tag
    char tag[strlen(BF_START_TAG)+1];
    memset(tag, 0, strlen(BF_START_TAG) + 1);
    strncpy(tag, buffer, strlen(BF_START_TAG));
    if (strcmp(tag, BF_START_TAG)) {
        fprintf(stderr, "BF: The buffer has no valid format.");
        free(bf);
        return NULL;
    }
    // parse the string
    u_int32_t i;
    int attr_i = 0;
    for (i = strlen(BF_START_TAG); i < len - strlen(BF_END_TAG); i++) {
        if (buffer[i] == sep) {
            attr_i ++;
            if (attr_i > 1) break;
        } else {
            attributes[attr_i] = 10 * attributes[attr_i] + (buffer[i] - '0');
        }
    }

    bf->hash_func_count = attributes[0];
    bf->size = attributes[1];
    bf->bit_string = calloc(bf->size, sizeof(uint8_t));
    memcpy(bf->bit_string, &(buffer[i+1]), bf->size);
    return bf;
}

// write data to a buffer from the given BF, separated by a user provided sep
void bf_to_buffer(BF bf, char **dest_buf, int *buf_len, char sep) {
    // format <bf><hash_func_count><sep><size><sep><bit_string></bf>

    // first determine the actual str size of the numeric fields
    int size_len, hash_func_len;
    char b[BUFSIZ];
    memset(b, 0, BUFSIZ);
    sprintf(b, "%u", bf->size);
    size_len = strlen(b);
    memset(b, 0, BUFSIZ);
    sprintf(b, "%u", bf->hash_func_count);
    hash_func_len = strlen(b);

    // then get the actual buffer size
    u_int32_t buffer_size =
        strlen(BF_START_TAG) * sizeof(char) +  // length of <bf>
        size_len + hash_func_len +             // length of the numeric fields
        bf->size +                             // length of the bit string
        strlen(BF_END_TAG) * sizeof(char)      // length of </bf>
        + 2;                                   // 2 separators
    char *buf = calloc(buffer_size, sizeof(char));

    sprintf(buf, "%s%u%c%u%c",  BF_START_TAG,
                                    bf->hash_func_count,
                                    sep,
                                    bf->size,
                                    sep);
    int write_at = buffer_size - bf->size - strlen(BF_END_TAG);
    memcpy(&(buf[write_at]), bf->bit_string, bf->size);
    memcpy(&(buf[write_at+bf->size]), BF_END_TAG, strlen(BF_END_TAG));
    *dest_buf = buf;
    *buf_len = buffer_size;
}

// write data to a file (given file descriptor) from the given BF, separated by a user provided sep
void bf_to_file(BF bf, int fd, char sep) {
    char *buf=NULL;
    int len=0;
    bf_to_buffer(bf, &buf, &len, sep);
    if (buf && len > 0) {
        write(fd, buf, len);
    }
}




/////////////////////////////////
// #include <stdio.h>
// // test main
// int main(void) {
//     BF bf = bf_create(16, 1000);

//     for (int i = 0; i < 1000; i++) {
//         if (!(i%7)){
//             char buf[100];
//             memset(buf, 0, 100);
//             sprintf(buf, "%d", i);
//             bf_insert(bf, buf);
//         }
//     }

//     // for (int i = 0; i < 1000; i++) {
//     //     char buf[100];
//     //     memset(buf, 0, 100);
//     //     sprintf(buf, "%d", i);
//     //     if (i % 7 == 0 && !bf_contains(bf, buf)) printf("%d, %f not in BF.\n", i, ((float)i/7.0));
//     // }
//     char *buf = NULL;
//     int len = 0;
//     bf_to_buffer(bf, &buf, &len, '$');
//     BF bf2 = bf_create_from_buffer(buf, len, '$');
//     if (memcmp(bf->bit_string, bf2->bit_string, bf->size)) {
//         puts("SKATA");
//     } else {puts("SUCCESS!");}

//     int fd = open("./test-bf", O_WRONLY | O_APPEND | O_CREAT, 0644);
//     bf_to_file(bf2, fd, '$');
//     close(fd);
//     bf_destroy(bf);
//     bf_destroy(bf2);
//     return 0;
// }