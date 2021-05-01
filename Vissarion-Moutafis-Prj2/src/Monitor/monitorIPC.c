#include "Monitor.h"

static void check_fds(int bufsiz, void *monitor, struct pollfd fds[], int nfd, int *active, MessageHandler handler, void *return_args[]) {
    if ((fds[0].revents & POLL_IN) == POLL_IN) {
        char *msg = NULL;
        int len = 0;
        int opcode = -1;
        read_msg(fds[0].fd, bufsiz, &msg, &len, &opcode);

        // check if the process has stoped transmitting
        if (opcode == MSGEND_OP) {
            // process do not write anymore so just leave
            (*active)--;
            return;
        }
        // some handling
        handler(monitor, msg, len, return_args);
        
        // end of handling

        // free the memory of msg
        free(msg);
    }
}

int monitor_get_response(int bufsiz, void *_monitor, MessageHandler handler, int process_id, int fd, void *return_args[]) {
    // in the monitor struct, the fd is the input 
    // fd where the travel monitor writes to.
    Monitor monitor = _monitor ? (Monitor)_monitor : NULL;
    struct pollfd fds[1];
    memset(fds, 0, sizeof(struct pollfd));
    // initialize fds
    fds[0].fd = fd;
    fds[0].events = POLL_IN;

    // number of active monitors
    int active = 1;
    while (active) {
        int ret = poll(fds, 1, 1);
        if (ret > 0) {
            check_fds(bufsiz, monitor, fds, 1, &active, handler, return_args);
        }
        if (ret < 0) {
            perror("poll");
            exit(1);
        }
    }

    return 1;
}