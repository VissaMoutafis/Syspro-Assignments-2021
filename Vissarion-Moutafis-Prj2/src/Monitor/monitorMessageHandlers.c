#include "Monitor.h"

static void init_msg(char dest[], char *msg, int msg_len) {
    memset(dest, 0, msg_len + 1);
    memcpy(dest, msg, msg_len);
}

void get_dirs(void *monitor, char *msg, int msg_len, void *return_args[]) {
    // monitor is irrelevant, 
    // return_args = {char *** dirs, int *len}
    // msg : <country>$<country>$...etc
    char buf[msg_len+1];
    init_msg(buf, msg, msg_len);

    char **dirs = NULL;
    dirs = parse_line(buf, (int *)(return_args[1]), SEP);
    *((char ***)return_args[0]) = dirs;
}

void get_init_stats(void *monitor, char *msg, int msg_len, void *return_args[]) {
    // monitor is irrelevant,
    // return_args = {u_int32_t *buffer_size, size_t *bloom_size}
    // msg : <buffer size (10 digs long)><bloom size (10 digs long)>

    char bufsiz[11], bloomsiz[11];
    memset(bufsiz, 0, 11);
    memset(bloomsiz, 0, 11);
    memcpy(bufsiz, msg, 10);
    memcpy(bloomsiz, msg+10, 10);

    *(u_int32_t*)return_args[0] = strtoul(bufsiz, NULL, 10);
    *(size_t*)return_args[1] = strtoul(bloomsiz, NULL, 10);
}

void get_query(void *monitor, char *msg, int msg_len, void *return_args[]) {
    // ret_args: {int expr_index, char *value}
    // msg: <10digs of expr_index><value>
    // first 10 bytes is the expr_index
    // the next msg_len - 10 bytes is the value (NOT terminated by a '\0')
    assert(msg_len >= 10);

    char expr_id_str[11];
    memset(expr_id_str, 0, 11);
    memcpy(expr_id_str, msg, 10);
    *((int *)return_args[0]) = atoi(expr_id_str);

    // now we have to read the value
    int value_len = msg_len-10+1;
    char *value = calloc(value_len, sizeof(char));
    memcpy(value, msg+10, value_len-1);
    *((char **)return_args[1]) = value;

    #ifdef DEBUG
    printf("get_query: expr_id: %d, value: %s\n", *((int *)return_args[0]), *((char **)return_args[1]));
    #endif
}