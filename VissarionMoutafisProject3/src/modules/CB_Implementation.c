/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/

#include "CB.h"

struct circular_buffer {
    Pointer *buffer_slots;  // actual arrays that each slot is a buffer
    int size;               // fixed size of the buffer
    int read_at;            // Index to read from. If none then is equal to -1
    int write_at;           // Index to write to. If none then is equal to -1
    ItemDestructor destroy; // destructor of the remaining indexes
};

/* Circular Buffer Implementation */

// create a circular buffer of size <size> with items that can be destroyed with
// <destroy>
CB cb_create(int size, ItemDestructor destroy) {
    CB cb = calloc(1, sizeof(*cb));

    // initialization 
    cb->destroy = destroy;
    cb->size = size > 0 ? size : DEF_CB_SIZE;
    cb->read_at = -1; // we do not have anything to read initialy
    cb->write_at = 0; // the buffer is empty so just write in the first position (index 0) 
    cb->buffer_slots = calloc(cb->size, sizeof(Pointer)); // allocate memory for the circular buffer

    return cb;
}

// Add an item in the circular buffer and get a pointer to it. On success we
// return <true>. If the buffer is full we will return <false>
bool cb_add(CB cb, Pointer item) {
    int write_at = cb->write_at;
    if (write_at >= 0 && !cb->buffer_slots[write_at]) {
        // if the next slot to write is empty then write into the slot
        cb->buffer_slots[write_at] = item;
        // init the <read_at> if needed
        if (cb->read_at < 0)
            cb->read_at = write_at; 
        // set the new <write_at> buffer
        write_at = (write_at + 1) % cb->size;
    } else {
        // we don't have anywhere to write to
        write_at = -1;
    }

    // update the <write_at> index in the header
    cb->write_at = write_at;

    // sanity check: READ AND WRITE CANNOT BE INVALID AT THE SAME TIME
    assert(cb->read_at >= 0 || cb->write_at >= 0);

    return write_at >= 0; // return true if the writing was successful only
}

// Get an item from the cb and return a void * to it and empty the spot. If we have nothing to read
// we just return NULL
Pointer cb_get(CB cb) {
    int read_at = cb->read_at;
    Pointer entry = NULL;
    if (read_at >= 0 && cb->buffer_slots[read_at]) {
        // if the slot is NOT empty we read it
        entry = cb->buffer_slots[read_at];
        // set the slot to empty
        cb->buffer_slots[read_at] = NULL;
        // check the <write_at> index and set it if needed
        if (cb->write_at < 0) 
            cb->write_at = read_at;
        // update the <read_at> index
        read_at = (read_at + 1) % cb->size;
    } else {
        // we don't have anything to read
        read_at = -1;
    }

    // update the <read_at> index in the header
    cb->read_at = read_at;
    
    // sanity check
    assert(cb->read_at >= 0 || cb->write_at >= 0);

    return entry; // just return the entry pointer
}

// destroy the circular buffer
void cb_destroy(CB cb) {

    if (cb->destroy) {
        for (int i = 0 ; i < cb->size; i++) {
            if (cb->buffer_slots[i])
                cb->destroy(cb->buffer_slots[i]);
        }
    }
    free(cb->buffer_slots);
    free(cb);
}

// test main
// #define TESTSIZE 6
// int main(int argc, char *argv[]) {
//     char *test_cases[TESTSIZE] = {"Hello", "World", "This", "is", "Vissa", "Mout"};
//     CB cb = cb_create(1, free);
//     int added = 0;
//     // add them all and then remove them all
//     puts("Adding Everything...");
//     for (int i = 0; i < TESTSIZE; i++) {
//         printf("Adding '%s'...", test_cases[i]);
//         char *t = calloc(strlen(test_cases[i]) + 1, sizeof(char));
//         strcpy(t, test_cases[i]);
//         if ( !cb_add(cb, t)) {
//             puts("Exiting because fail.");
//             free(t);
//             break;
//         } else {
//             puts("SUCCESS");
//         }
//         // remove it straight away
//         char *ret = NULL;
//         while (i%2 && (ret = (char *)cb_get(cb)) != NULL) {
//             printf("Got '%s'\n", ret);
//             free(ret);
//         }
//         added ++;
//     }

//     puts("Getting everything from cb");
//     char *ret = NULL;
//     while ((ret = (char*)cb_get(cb)) != NULL) {
//         printf("Got '%s'\n", ret);
//         free(ret);
//     }

//     puts("Adding the rest of the test cases");
//     for (int i = added; i < TESTSIZE; i++) {
//         printf("Adding '%s'...", test_cases[i]);
//         char *t = calloc(strlen(test_cases[i]) + 1, sizeof(char));
//         strcpy(t, test_cases[i]);
//         if (!cb_add(cb, t)) {
//             puts("Exiting because fail.");
//             free(t);
//             break;
//         } else {
//             puts("SUCCESS");
//         }
//         added++;
//     }

//     puts("Getting everything from cb");
//     ret = NULL;
//     while ((ret = (char *)cb_get(cb)) != NULL) {
//         printf("Got '%s'\n", ret);
//         free(ret);
//     }

//     cb_destroy(cb);
// }