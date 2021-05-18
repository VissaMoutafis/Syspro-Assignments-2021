/**
*	Syspro Project 2
*	 Written By Vissarion Moutafis sdi1800119
**/
 
/*
** Double Linked List Implementation
** Written by Vissarion Moutafis sdi1800119 
** for OS project 1 fall 2020
*/
#include <stdio.h>

#include "List.h"

struct listnode {
    Pointer entry;
    ListNode next;
    ListNode prev;
};

struct list {
    ListNode head;
    ListNode tail;
    int count;
    ItemDestructor itemDestructor;
    Compare compare;
};

ListNode create_node(Pointer entry, ListNode next, ListNode prev) {
    ListNode n = malloc(sizeof(*n));
    n->entry = entry;
    n->next = next;
    n->prev = prev;

    return n;
}


//List methods

// Creates a list.
List list_create(Compare compare, ItemDestructor itemDestructor) {
    assert(compare);
    
    List l = malloc(sizeof(*l)); // allocate the proper memory
    
    // Initialization
    l->count = 0;
    l->head = LIST_EOF;
    l->tail = LIST_EOF;
    l->compare = compare;
    l->itemDestructor = itemDestructor;

    return l;
}

// Checks if the list is empty
bool list_empty(List list) {
    assert(list);
    return list->count == 0 && list->head == LIST_EOF && list->tail == LIST_EOF;
}

// Get the first entry of the list
ListNode list_get_head(List list) {
    assert(list);
    return list->head;
}

ListNode list_get_tail(List list) {
    assert(list);
    return list->tail;
}

ListNode list_get_next(List list, ListNode node) {
    assert(list);
    return node ? node->next : NULL;
}

ListNode list_get_prev(List list, ListNode node) {
    assert(list);
    return node ? node->prev : NULL;
}

int list_len(List list) {
    assert(list);
    return list->count;
}

// Finds the node and returns it, otherwise returns NULL.
ListNode list_find(List list, Pointer entry) {
    assert(list);

    ListNode cur = list_get_head(list);

    // search for the node
    while (cur != LIST_EOF) {
        // save the current entry
        Pointer cur_entry = list_node_get_entry(list, cur);
        
        // compare the entry with the current entry and
        // if its the entry we are looking for then return 
        // the node that 'cur' points to 
        if (list->compare(cur_entry, entry) == 0)
            return cur;
        cur = cur->next;
    }

    // if we reached this far then the search was unsuccessful
    return LIST_EOF;
}

// Inserts an entry in the list.
// If the last is false it is inserted at the beginning, else it is inserted at the end.
void list_insert(List list, Pointer entry, bool last) {
    /*
        The logic is the following:
        1. if last is true then insert at the end in O(1)
        2. if last is false then isert at the start  in O(1)
    */
    assert(list);
    ListNode new_node = create_node(entry, NULL, NULL);

    if (list_empty(list)) {
        // if the list is empty
        list->head = list->tail = new_node;

    } else {
        // if the list is not empty

        if (last) {
            // insert at the end
            list->tail->next = new_node;
            new_node->prev = list->tail;
            list->tail = new_node; 

        } else {
            // insert at the beginning
            list->head->prev = new_node;
            new_node->next = list->head;
            list->head = new_node; 
        }
    }
    list->count ++;
}

// Delete a specific entry 
bool list_delete(List list, Pointer entry,  bool delete_entry, Pointer *old_entry) {
    assert(list);
    
    ListNode node = list_find(list, entry);
    if (node) {
        list->count--;
        *old_entry = (delete_entry == false) ? list_node_get_entry(list, node) : NULL;
        ListNode prev, next;
        prev = node->prev;
        next = node->next;

        // detach the node from the list
        if (next)
            next->prev = prev;
        if (prev)
            prev->next = next;

        // make sure that the head and tail pointers are correctly adjusted 
        if (node == list->head)
            list->head = node->next;
        if (node == list->tail)
            list->tail = node->prev;

        // delete the node and the node's entry if it's ordered by the caller
        if (delete_entry) {
            if (list->itemDestructor)
                list->itemDestructor(node->entry);
            *old_entry = NULL;
        }
        free(node);
        return true;
    }
    
    // if we reached this far something failed
    return false;
}

// Sets a new itemDestructor for the list.
void list_set_destructor(List list, ItemDestructor itemDestructor) {
    assert(list);
    list->itemDestructor = itemDestructor;
}

ItemDestructor list_get_destructor(List list) {
    return list ? list->itemDestructor : NULL;
}

// De-allocate the memory allocated for the list.
void list_destroy(List *list) {
    ListNode cur, next;
    cur = (*list)->head;

    while (cur) {
        //make sure you keep the next node to traverse
        next = cur->next;

        // free the memory
        if ((*list)->itemDestructor) {
            (*list)->itemDestructor(cur->entry);
        }
        free(cur);

        // get to the next node
        cur = next;
    }

    free(*list);
    *list = NULL;
}

// List Node methods

// Getter for the node entry
Pointer list_node_get_entry(List list, ListNode node) {
    return node != NULL ? node->entry : NULL;
}

// Set a new entry for a specific node.
void list_node_set_entry(List list, ListNode node, Pointer new_entry, Pointer *old_entry) {
    assert(node);
    assert(list);
    assert(new_entry);
    assert(old_entry);
    *old_entry = node->entry;
    node->entry = new_entry;
}

// Additional-Extra Methods for OS project 1

void list_print(List list, Visit visit) {
    ListNode cur = list->head;

    while (cur != LIST_EOF) {
        visit(cur->entry);
        cur = cur->next;
    }
}

// Pointer list_find_min(List list, Compare compare) {
//     ListNode n = list->head;
//     Pointer cur, min;
//     min = n->entry;

//     while (n != LIST_EOF) {
//         cur = n->entry;
//         min = compare(cur, min) < 0 ? cur : min;
//         n = n->next;
//     }

//     return min;
// }

// Pointer list_find_max(List list, Compare compare) {
//     ListNode n = list->head;
//     Pointer cur, max;
//     max = n->entry;

//     while (n != LIST_EOF) {
//         cur = n->entry;
//         max = compare(cur, max) > 0 ? cur : max;
//         n = n->next;
//     }

//     return max;
// }

// void list_insert_sorted(List list, Pointer entry, Compare compare) {
//     ListNode c = list->head; 
    
//     while (c != LIST_EOF && compare(c->entry, entry) > 0) {
//         // while c is in the list and the c->entry > entry
//         c = c->next;
//     }
//     // c will be the node that is next to the new entry node
//     ListNode new_node = create_node(entry, c, NULL);

//     if (c == list->head)
//         list->head = new_node;

//     if (c != NULL){ 
//         new_node->prev = c->prev;

//         if (c->prev)
//             c->prev->next = new_node;

//         c->prev = new_node;
//     } else {
//         // we add it at the end of the list
//         new_node->prev = list->tail;
//         if (list->tail)
//             list->tail->next = new_node;
//         list->tail = new_node;
//     }

//     list->count ++;
// }

// List list_get_top_n(List list, Compare compare, int n) {
//     ListNode cur= list->head;
//     cur = list->head;
//     List top_n_th = list_create(list->compare, NULL);
//     // while we reach the n_th top or the list ends
//     while(cur != NULL) {
//             list_insert_sorted(top_n_th, cur->entry, compare);
//             cur = cur->next;
//     }
//     int to_delete = list->count - n;

//     if (to_delete > 0 && n >= 0) {
//         while(to_delete-- && !list_empty(top_n_th)) {
//             ListNode tail = top_n_th->tail;
//             if (top_n_th->tail){
//                 top_n_th->tail = tail->prev;
//                 if (top_n_th->tail)
//                     top_n_th->tail->next = NULL;
//             }
//             if (top_n_th->tail == NULL)
//                 top_n_th->head = top_n_th->tail = NULL;
//             free(tail);
            
//         }
//     }
//     return top_n_th;
// }




// int cmp(void *i1, void *i2) {
//     return *(int*)i1 - *(int*)i2;
// }
// void *create(int i) {
//     int *p = malloc(sizeof(int));
//     *p = i;
//     return (void *)p;
// }
// void visit(void *i) {
//     printf("%d ", *(int*)i);
// }
// int main(void) {
//     List l = list_create(cmp, free);
//     int MAX = 9999;
//     if (list_len(l)) puts("ERROR len");
//     if (!list_empty(l)) puts("ERROR empty");
//     if (list_get_head(l)) puts("ERROR get_head");
//     for (int i = 0; i < MAX; i++) {
//         list_insert(l, create(i), true);       
//     }

//     for (int i = 0; i < MAX; i ++) {
//         void * item = create(i);
//         if (!list_find(l, item))
//             puts("ERROR after insert");
//         free(item);
//     }

//     for (int i = MAX; i < 2*MAX; i++) {
//         void *item = create(i);
//         if (list_find(l, item)) puts("ERROR find");
//         free(item);
//     }

//     for (int i = 0; i <  MAX; i+=2) {
//         void *item = create(i);
//         void *old;
//         list_delete(l, item, true, &old);
//         free(item);
//     }

//     for (int i = 0; i < MAX; i+=2) {
//         void *item = create(i);
//         if (list_find(l, item))
//             printf("ERROR after delete %d\n", *(int*)list_node_get_entry(l, list_find(l, item)));
//         free(item);
//     }
//     for (int i = 0; i < MAX; i ++) {
//         void *item = create(i);
//         void *old;
//         list_delete(l, item, true, &old);
//         free(item);
//     }

//     for (int i = 0; i < MAX; i ++) {
//         void *item = create(i);
//         if (list_find(l, item))
//             printf("ERROR after delete %d\n",
//                    *(int *)list_node_get_entry(l, list_find(l, item)));
//         free(item);
//     }

//     if (list_len(l)) puts("ERROR len");
//     if (!list_empty(l)) puts("ERROR empty");
//     if (list_get_head(l)) puts("ERROR get_head");
//     for (int i = 0; i < MAX; i++) {
//         list_insert(l, create(i), true);
//     }

//     for (int i = 0; i < MAX; i++) {
//         void *item = create(i);
//         if (!list_find(l, item)) puts("ERROR after insert");
//         free(item);
//     }

//     for (int i = MAX; i < 2 * MAX; i++) {
//         void *item = create(i);
//         if (list_find(l, item)) puts("ERROR find");
//         free(item);
//     }

//     for (int i = 0; i < MAX; i += 2) {
//         void *item = create(i);
//         void *old;
//         list_delete(l, item, true, &old);
//         free(item);
//     }

//     for (int i = 0; i < MAX; i += 2) {
//         void *item = create(i);
//         if (list_find(l, item))
//             printf("ERROR after delete %d\n",
//                    *(int *)list_node_get_entry(l, list_find(l, item)));
//         free(item);
//     }
//     for (int i = 0; i < MAX; i++) {
//         void *item = create(i);
//         void *old;
//         list_delete(l, item, true, &old);
//         free(item);
//     }

//     for (int i = 0; i < MAX; i++) {
//         void *item = create(i);
//         if (list_find(l, item))
//             printf("ERROR after delete %d\n",
//                    *(int *)list_node_get_entry(l, list_find(l, item)));
//         free(item);
//     }
//     list_destroy(&l);
//     return 0;
// }