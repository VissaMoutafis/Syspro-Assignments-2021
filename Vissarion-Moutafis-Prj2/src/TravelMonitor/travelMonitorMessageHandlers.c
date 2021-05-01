#include "TravelMonitor.h"

void get_bf_from_child(void *monitor, char *msg, int msg_len, void *return_args[]) {
    // just print it to see it works
    char buf[msg_len+1];
    memset(buf, 0, msg_len+1);
    memcpy(buf, msg, msg_len);
    printf("Len: %d -- ", msg_len);
    for (int i =0; i < msg_len; i++)
        if (msg[i])
            putchar(msg[i]);
        else 
            putchar('-');
}