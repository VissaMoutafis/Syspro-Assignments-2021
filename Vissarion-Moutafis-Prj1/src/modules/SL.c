#include "SL.h"

struct skip_list {
    SLNode head;
    SLNode tail;
    Compare compare;
    ItemDestructor item_destructor;
    u_int32_t max_level;
    float p;
};

struct skip_list_node {
    Pointer entry;          // the entry pointer
    SLNode *next, *prev;    // the array of next and prev pointers in every level
};

static SLNode create_slnode(Pointer entry, u_int32_t levels) {
    SLNode node = calloc(1, sizeof(*node));

    // init 
    node->entry = entry;
    node->next = calloc(levels, sizeof(SLNode));
    node->prev = calloc(levels, sizeof(SLNode));

    return node;
}

