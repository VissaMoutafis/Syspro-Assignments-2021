/**
*	Syspro Project 2
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#include "Monitor.h"

static void check_fds(int bufsiz, void *monitor, struct pollfd fds[], int nfd, int *active, MessageHandler handler, void *return_args[]) {
    if ((fds[0].revents & POLL_IN) == POLL_IN) {
        char *msg = NULL;
        int len = 0;
        int opcode = -1;
        read_msg(fds[0].fd, bufsiz, &msg, &len, &opcode);

        // check if the process has stoped transmitting or if we just read a symbolic packet
        if (opcode == MSGEND_OP || opcode == SYN_OP || opcode == ACK_OP) {
            if (handler && (opcode == SYN_OP || opcode == ACK_OP))
                handler(monitor, opcode, msg, len, return_args);
            // process do not write anymore so just leave
            (*active)--;
            return;
        }
        // some handling
        if (handler)
            handler(monitor, opcode, msg, len, return_args);
        
        // end of handling

        // free the memory of msg
        if (msg && len > 0)
            free(msg);
    }
}

int monitor_get_response(int bufsiz, void *_monitor, MessageHandler handler, int fd, void *return_args[]) {
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
    while (active > 0) {
        int ret = poll(fds, 1, 1);
        if (ret < 0) {
            // break on signal 
            if (errno == EINTR) return 0;
            perror("poll in monitor");
            exit(1);
        }
        if (ret > 0) {
            check_fds(bufsiz, monitor, fds, 1, &active, handler, return_args);
        }
    }

    return 1;
}