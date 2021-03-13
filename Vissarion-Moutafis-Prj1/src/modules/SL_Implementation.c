/*
** Skip List Implementation
** Implemented by Vissarion Moutafis sdi1800119
*/
#include "SL.h"
#include <stdio.h>

struct skip_list {
    SLNode head;
    Compare compare;
    ItemDestructor item_destructor;
    int max_level;
    float p;
};

struct skip_list_node {
    Pointer entry;          // the entry pointer
    SLNode *next;           // the array of next and prev pointers in every level
};

// Function to create a node
static SLNode create_slnode(Pointer entry, int levels) {
    assert(levels);

    SLNode node = calloc(1, sizeof(*node));

    // init 
    node->entry = entry;
    node->next = calloc(levels, sizeof(SLNode));

    return node;
}

// Function to destroy an sl node
static void destroy_slnode(SL sl, SLNode node) {
    assert(sl);
    assert(node);
    
    // if there exists an entry and the user provided the struct with an item destructor, proceed to destroy entry
    if (node->entry && sl->item_destructor)
        sl->item_destructor(node->entry);
    
    free(node->next);
    free(node);
}

// Function that realized the probabilistic character of the struct
static bool flip_coin(float p) {
    // p: is the probability of returning true
    
    // get a random number 
    float p_hat = ((float)rand())/((float)RAND_MAX);

    // return true only if the probability is in the desired interval [0,p]
    return (p_hat-p <= 0.0);
}

// Function to determine the height of a node (probabilisticaly)
static int get_node_height(SL sl) {
    assert(sl);
    
    // increase the height of a node based on flip coin procedure
    int height = 1;
    while (height < sl->max_level && flip_coin(sl->p))
        height++;

    return height;
}

// Function to insert a node to the list and update the list's links
static void sl_update(SL sl, SLNode node, SLNode *update_list, int node_height) {
    assert(sl);

    // update the structure of sl based on the update list
    for (int level = 0; level < node_height; level++) {
        // for every level from 0 to node_height 
        // update the links based on the update_list 
        
        // the next node in the current level
        SLNode next = update_list[level]->next[level];

        // insert the node in the current level
        update_list[level]->next[level] = node;
        // update the node's next list
        node->next[level] = next;
    }
}

// Function that will be deleting node and updating the links of the struct
static bool delete_slnode(SL sl, SLNode last, SLNode node, bool destroy_entry, Pointer *old_entry, int height) {
    assert(sl);
    assert(node);

    SLNode prev = last;
    // since we have the last node we will traverse all the lists from this height till we hit zero level
    for (int level = height-1; level >= 0; level--) {
        
        // determine which is the prev-to-deleted node in the list of the current level
        while (prev->next[level] != node)
            prev = prev->next[level];
        
        // dettach the node from the list
        prev->next[level] = node->next[level];
        // update the last node in order to 
    }

    if (!destroy_entry) {
        *old_entry = node->entry;
        node->entry = NULL;
    }

    // destroy the node
    destroy_slnode(sl, node);

    return true;
}

// Creator function of the skip list.
SL sl_create(Compare compare, ItemDestructor itemDestructor, int max_level, float leveled_nodes_percent) {
    // basic assertions of the lists struct
    assert(compare);
    assert(max_level > 0);
    assert(leveled_nodes_percent >= 0.0);

    // allocate the memory and initialize the fields to 0x0 (NULL)
    SL sl = calloc(1, sizeof(*sl));

    // init
    sl->compare = compare;
    sl->item_destructor = itemDestructor;
    sl->max_level = max_level;
    sl->p = leveled_nodes_percent;
    sl->head = create_slnode(NULL, max_level); // insert a dummy node that will be the starting node of each level (complementary)

    return sl;
}

// The insert function of the skip list. Insert entry into sl. If the entry is already in the sl 
// we decide whether to replace or not by setting the replace flag properly 
void sl_insert(SL sl, Pointer entry, bool replace, Pointer *old_entry) {
    assert(sl);
    *old_entry = NULL;

    // we must find a place to enter the node
    SLNode node = sl->head->next[sl->max_level-1];
    SLNode update_list[sl->max_level];
    for (int i = 0; i < sl->max_level; i++)
        update_list[i] = sl->head;

    // we begin from the top level and search for a node, till we find it or we reach the bottom level
    for (int level = sl->max_level-1; level >= 0; level--) {
        // loop till you reach the end of the list in the current level 
        // or till you find an entry with larger
        int res=-1;
        while (node && (!node->entry || (res = sl->compare(entry, node->entry)) <= 0)) {
            // we found a node with the same entry
            if (res == 0 ) {
                // If the user wants to replace the node's entry then replace it
                // and provide the user with the old_entry
                if (replace) {
                    *old_entry = node->entry;
                    node->entry = entry;
                }
                return;
            }

            update_list[level] = node;
            node = node->next[level];
        }
        node = update_list[level];
    }

    // At this point, we will append a new_node 
    // First determine its height.
    int height = get_node_height(sl);
    // Create a node with as many levels as its height
    SLNode new_node = create_slnode(entry, height);
    // Update the list structure 
    sl_update(sl, new_node, update_list, height);
}

// The deletion function of the skip list. Delete entry from sl. If the flag destroy_entry is set to true 
// then the entry is destroyed and old_entry is null, otherwise the old entry points to the deleted nodes entry. 
// If the delete is successful return true, else return false
bool sl_delete(SL sl, Pointer entry, bool destroy_entry, Pointer *old_entry) {
    assert(sl);
    assert(entry);
    *old_entry = NULL;

    SLNode node = sl->head->next[sl->max_level-1];
    SLNode last = sl->head;
    // we begin from the top level and search for a node, till we find it or we reach the bottom level
    for (int level = sl->max_level-1; level >= 0; level--) {
        // loop till you reach the end of the list in the current level 
        // or till you find an entry with larger
        int res=-1;
        while (node && (!node->entry || (res = sl->compare(entry, node->entry)) <= 0)) {
            // we found the node
            if (res == 0)
                return delete_slnode(sl, last, node, destroy_entry, old_entry, level+1);

            last = node;
            node = node->next[level];
        }
        node = last;
    }

    return false;
}

// Search for an entry in the sl. If found return the entry. 
// Otherwise return NULL 
Pointer sl_search(SL sl, Pointer search_entry) {
    assert(sl);
    assert(search_entry);

    SLNode node = sl->head->next[sl->max_level-1];
    SLNode last = sl->head;
    // we begin from the top level and search for a node, till we find it or we reach the bottom level
    for (int level = sl->max_level-1; level >= 0; level--) {
        // loop till you reach the end of the list in the current level 
        // or till you find an entry with larger
        int res=-1;
        while (node && (!node->entry || (res = sl->compare(search_entry, node->entry)) <= 0)) {
            // we found the node
            if (res == 0) {
                return node->entry;
            }
            last = node;
            node = node->next[level];
        }
        node = last;
    }

    return NULL;
}

// Function to free all the memory of the Skip List. WARNING the sl will be pointing to trash memory after this action
void sl_destroy(SL sl) {
    SLNode node, next;
    node = sl->head;
    next = NULL;
    
    // traverse the list and delete each and every node
    while (node) {
        // keep the next node
        next = node->next[0];
        // destroy the node
        destroy_slnode(sl, node);
        // proceed to next node
        node = next;
    }

    // free the list header
    free(sl);
}

void sl_apply(SL sl, Apply visit) {
    assert(sl);
    SLNode node = sl->head->next[0];

    while (node->next[0]) {
        assert(node->entry);
        visit(node->entry);
        node = node->next[0];
    }
    assert(node->entry);
    visit(node->entry);
}

// Function to set a new destructor for the sl. The previous one is saved in prev_destructor pointer
void sl_set_destructor(SL sl, ItemDestructor new_destructor, ItemDestructor *prev_destructor) {
    assert(sl);
    assert(prev_destructor);

    *prev_destructor = sl->item_destructor;
    sl->item_destructor = new_destructor;
}



// test main

// int cmp(void *i1, void *i2) {
//     return *(int*)i1 - *(int*)i2;
// }

// void destroy(void *i) {
//     free(i);
// }

// int main(void) {

//     SL sl = sl_create(cmp, destroy, 17, 0.5);
//     int MAX=100000;

//     // TESTING THAT INSERT AND FIND WORK OUT

//     for (int i = 0; i < MAX; i ++) {
//         int *p = malloc(sizeof(int));
//         void  *p_hat;
//         *p = i;
//         // printf("Inserting %d.\n", i);
//         sl_insert(sl, p, false, &p_hat);
//         if (!sl_search(sl, p))
//             printf("Problem during inserting %d.\n", i);
        
//     }

//     // TESTING DELETE IN DIFFERENT SCENARIOS
//     for (int i = 0; i < MAX; i++) {
//         int *p = malloc(sizeof(int));
//         void *p_hat=NULL;
//         *p = i;

//         if (i < MAX / 2 && !sl_delete(sl, p, true, &p_hat))
//             printf("Problem during deleting %d.\n", i);

//         if (i >= MAX / 2 && !sl_delete(sl, p, false, &p_hat)) 
//             printf("Problem during deleting %d.\n", i);
        
//         if (p_hat)
//             free(p_hat);
        
//         *p = MAX+i;

        
//         if (sl_delete(sl, p, true, &p_hat)) 
//             printf("IMPORTANT Problem: Trying to delete %d, while it does not exist!!!\n", *p);
        
//         free(p);
//     }

//     // TRYING TO FIND DELETED VALUES

//     for (int i = 0; i < MAX; i++) {
//         int *p = malloc(sizeof(int));
//         *p = i;
//         if (sl_search(sl, p)) printf("Problem during deletion of %d.\n", i);
//         free(p);
//     }


//     puts("RE INSERTING ALL ELEMENTS");
//     // TESTING INSERT AFTER DELETING EVERYTHING
//     for (int i = 0; i < 2*MAX; i++) {
//         int *p = malloc(sizeof(int));
//         void *p_hat;
//         *p = i;
//         sl_insert(sl, p, true, &p_hat);
//         free(p_hat);
//     }

//     for (int i = 0; i < 2*MAX; i++) {
//         int *p = malloc(sizeof(int));
//         *p = i;
//         if (!sl_search(sl, p)) printf("Problem inserting deletion of %d.\n", i);
//         free(p);
//     }

//     sl_destroy(sl);

//     return 0;
// }
