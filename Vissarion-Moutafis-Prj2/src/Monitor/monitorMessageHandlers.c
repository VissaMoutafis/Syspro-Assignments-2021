#include "Monitor.h"

void get_dirs(void *monitor, char *msg, int msg_len, void *return_args[]) {
    // monitor is irrelevant, return_args = {char *** dirs, int *len}
    char buf[msg_len+1];
    memset(buf, 0, msg_len+1);
    memcpy(buf, msg, msg_len);

    char **dirs = NULL;
    dirs = parse_line(buf, (int *)(return_args[1]), SEP);
    *((char ***)return_args[0]) = dirs;
}