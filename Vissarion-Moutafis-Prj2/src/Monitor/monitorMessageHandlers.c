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