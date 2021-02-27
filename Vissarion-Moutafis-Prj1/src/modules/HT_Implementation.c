/*  Hash Table Implementation by Vissarion Moutafis
**  Using Open Addressing with Double Hashing
*/
#include <stdio.h>
#include "HT.h"

struct hashtable {
    Pointer *array;                 // the array that holds the keys
    Compare compare;                // compare function for the searching
    Hash_Func hash;                 // the hash function
    ItemDestructor itemDestructor;  // the item destructor
    u_int32_t size;                    // current size of the table
    u_int32_t item_count;              // current count of the keys in the table
};

char dummy = 'c';


// Utilities
static u_int32_t probing_hash(u_int32_t key_hash, u_int32_t table_size) {
    // The main idea is that we have a table with table_size being a power of 2
    // so the probing hash should return a value
    // between 1 and table_size that is an odd number. We also hope for a diversity in the probing returned

    if (key_hash < table_size / 2)
        return ((table_size / 2) % 2 == 0) ?  : table_size / 2;
    else
        // we know for sure that (table_size - 1)  and (table_size) are coprimes
        return table_size - 1;
}

static bool is_dummy(Pointer table_entry) {
    return table_entry == &dummy;
}

// define if a slot is empty or not (trivial)
static bool is_empty_slot(Pointer table_entry) { return table_entry == NULL; }

// Hash table rehashing function
static void rehash(HT hash_table) {
    // Create the new array
    u_int32_t old_size = hash_table->size;
    u_int32_t new_size = 2 * old_size;

    // set the array pointers
    Pointer *old_array = hash_table->array;
    Pointer *new_array = calloc(new_size, sizeof(Pointer));  // initialize every block with NULL
    for (u_int32_t i = 0; i < new_size; i++) new_array[i] = NULL;
    // set the new table of the hash table
    hash_table->array = new_array;
    // Set the new size
    hash_table->size = new_size;

    // Copy the old to the new larger array
    for (u_int32_t i = 0; i < old_size; ++i) {
        // for every element in the previous hash table, insert it to the new
        // one
        Pointer old_key;
        
        if (!is_empty_slot(old_array[i]) && !is_dummy(old_array[i]))
            ht_insert(hash_table, old_array[i], false, &old_key);
    }

    // free the memory of the old array
    free(old_array);
}

static bool find_key(Pointer *array, u_int32_t size, Compare compare, Pointer key, u_int32_t key_hash, u_int32_t probe_step, u_int32_t *pos) {
    assert(array != NULL);
    assert(compare != NULL);
    

    // initialize the key_hash carefully
    key_hash %= size;

    // while the entry is not empty and the keys are different:
    while (is_dummy(array[key_hash]) || (!is_empty_slot(array[key_hash]) && compare(key, array[key_hash]) != 0))
        key_hash = (key_hash + probe_step) % size;
    

    // If we actually found the key, then we must set the pos pointer to be
    // current key hash and return true.
    // If we didn't found the key, we save *the first empty position* to pos
    *pos = key_hash;
    return  (!is_empty_slot(array[key_hash]) && compare(key, array[key_hash]) == 0) ;
}

// Hash Table Methods Implementation

HT ht_create(Compare compare, Hash_Func hash_func, ItemDestructor itemDestructor) {
    HT ht = malloc(sizeof(*ht));

    // initialize the header's fields
    ht->hash = hash_func;
    ht->compare = compare;
    ht->itemDestructor = itemDestructor;
    ht->size = INITIAL_HT_SIZE;
    ht->item_count = 0;

    // The table will be a 1-D array of Pointer* ( aka (void*)* ). Use calloc to initialize every pointer to NULL
    ht->array = calloc(ht->size, sizeof(Pointer));

    return ht;
}

void ht_insert(HT hash_table, Pointer key, bool replace, Pointer *old_key) {
    // First hash the entry and find a suitable probe step
    u_int32_t key_hash = hash_table->hash(key);
    u_int32_t probe_step = probing_hash(key_hash, hash_table->size);
    u_int32_t position;
    *old_key = NULL;
    // Now we must add the key
    // If the key is already in the table then we change the entry
    if (find_key(hash_table->array, hash_table->size, hash_table->compare, key, key_hash, probe_step, &position)) {
        // position = 'the index of the 'key' that exist already in the table
        // so we change the entry if we want to replace it
       if (replace) {
            // save the old key to the respective pointer
            *old_key = hash_table->array[position];
            // and we must add the new key in 'position'
            hash_table->array[position] = key;
       }
    } else {
        // case that the key is not already in the hash table
        // so we just add it and increase the item_count
        hash_table->array[position] = key;
        hash_table->item_count++;
    }

    // Check if there's a need for re-hashing
    // if the item count surpasses half the size of the table then we need to
    // rehash it
    if (hash_table->size < 2 * hash_table->item_count) rehash(hash_table);
}

bool ht_contains(HT hash_table, Pointer key, Pointer *key_ptr) {
    // First hash the entry and find a suitable probe step
    assert(key);
    assert(hash_table);
    assert(hash_table->array);


    u_int32_t key_hash = hash_table->hash(key);
    u_int32_t probe_step = probing_hash(key_hash, hash_table->size);
    u_int32_t position;

    // Now we must find the entry
    // we will use the find_key function
    if (find_key(hash_table->array, hash_table->size, hash_table->compare, key, key_hash, probe_step, &position)) {
        // since we found the key  we must set the *key_ptr and return true
        *key_ptr = hash_table->array[position];
        return true;
    }

    // we did not find the key
    *key_ptr = NULL;
    return false;
}

void ht_delete(HT hash_table, Pointer key, bool delete_key, Pointer *key_ptr) {
    // First hash the entry and find a suitable probe step
    u_int32_t key_hash = hash_table->hash(key);
    u_int32_t probe_step = probing_hash(key_hash, hash_table->size);
    u_int32_t position;
    // initialize the pointer and change it only if delete_key = false, to point to the now deleted entry's key
    *key_ptr = NULL;

    if (find_key(hash_table->array, hash_table->size, hash_table->compare, key, key_hash, probe_step, &position)) {
        // we found the key
        if (delete_key == true) {
            // destroy the key
            if (hash_table->itemDestructor)
                hash_table->itemDestructor(hash_table->array[position]);
        } else {
            // we don't want to delete the key
            *key_ptr = hash_table->array[position];
        }

        // Make sure that we make the index null when we done
        hash_table->array[position] = &dummy;
        hash_table->item_count--;
    }
}

void ht_print_keys(HT hash_table, Visit visit_key) {
    for (u_int32_t i = 0; i < hash_table->size; ++i) {
        // if the slot is not empty then visit the key and display it.
        // The diplay manner is determined by the caller
        if (!is_empty_slot(hash_table->array[i]))
            visit_key(hash_table->array[i]);
    }
}

// Simple function to free the memory
void ht_destroy(HT hash_table) {
    for (u_int32_t i = 0; i < hash_table->size; ++i) {
        // if the slot is not empty then free the memory
        if (!is_empty_slot(hash_table->array[i]) && !is_dummy(hash_table->array[i]))
            if (hash_table->itemDestructor != NULL)
                hash_table->itemDestructor(hash_table->array[i]);
    }

    free(hash_table->array);
    free(hash_table);
}

// test main
int cmp(void *i1, void *i2) { return *(int *)i1 - *(int *)i2; }


u_int32_t hash(void *i) {
    // return *(int*)i;
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);
    sprintf(buf, "%u", *(u_int32_t*)i);
    return hash_i(buf, *(u_int32_t *)i);
}

void * create(int i ) {
    int *p = malloc(sizeof(int));
    *p = i;
    return (void*)p;
}

// int main(void) {
//     HT ht = ht_create(cmp, hash, free);
//     HT ht2 = ht_create(cmp, hash, free);

//     int MAX = 9999;

//     puts("TESTING SIMPLE INSERT");
//     for (int i = 0; i < MAX; i++) {
//         Pointer old;
//         Pointer key = create(i);
//         ht_insert(ht, key, false, &old);
//         if (!ht_contains(ht, key, &old)) printf("Problem in insertion of %d.\n", i);
//     }

//     puts("TESTING DELETE");
//     for (int i = 0; i <= MAX; i+=2) {
//         Pointer old;
//         Pointer key = create(i);
        
//         ht_delete(ht, key, true, &old);
//         if (ht_contains(ht, key, &old))
//             printf("Problem in deleting of %d.\n", i);
//         free(key);
//     }

//     puts("TESTING SIMPLE INSERT");
//     for (int i = 0; i < MAX; i++) {
//         Pointer old;
//         Pointer key = create(i);
//         ht_insert(ht2, key, false, &old);
//         if (!ht_contains(ht2, key, &old))
//             printf("Problem in insertion of %d.\n", i);
//     }

//     puts("TESTING DELETE");
//     for (int i = 0; i <= MAX; i += 2) {
//         Pointer old;
//         Pointer key = create(i);

//         ht_delete(ht2, key, true, &old);
//         if (ht_contains(ht2, key, &old))
//             printf("Problem in deleting of %d.\n", i);
//         free(key);
//     }

//     puts("TESTING REPLACE");
//     for (int i = 0; i < MAX; i++) {
//         Pointer old;
//         Pointer key = create(i);

//         if (i % 2 == 0 && ht_contains(ht, key, &old)) {
//             puts("SKATA");
//             printf("%d %u\n", i, hash(&i) % 65536);
//             exit(1);
//         }
//         ht_insert(ht, key, true, &old);
//         free(old);
//         if (!ht_contains(ht, key, &old))
//             printf("Problem in insertion of %d.\n", i);
//     }

//     puts("TESTING REPLACE");
//     for (int i = 0; i < MAX; i++) {
//         Pointer old;
//         Pointer key = create(i); 

//         if (i % 2 == 0 && ht_contains(ht2, key, &old)){ puts("SKATA");
//             printf("%d %u\n", i, hash(&i) % 65536);
//             exit(1);
//         }
//         ht_insert(ht2, key, true, &old);
//         free(old);
//         if (!ht_contains(ht2, key, &old))
//             printf("Problem in insertion of %d.\n", i);
//     }

//     ht_destroy(ht);
//     ht_destroy(ht2);
//     return 0;
// }
