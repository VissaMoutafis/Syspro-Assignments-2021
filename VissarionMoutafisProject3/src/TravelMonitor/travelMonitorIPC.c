/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#include "TravelMonitor.h"

static void intialize_fds(void *_monitor, int nfd, struct pollfd fds[], int sockfd, int sockfds[]) {
    TravelMonitor monitor = (TravelMonitor)_monitor;
    // initalize the fds 
    memset(fds, 0, nfd*sizeof(struct pollfd));

    if (sockfd < 0) {
        // we set all the fd's in the manager monitor
        for (int i = 0; i < nfd; i++) {
            MonitorTrace t;
            memset(&t, 0, sizeof(Trace));
            if (!monitor_manager_get_at(monitor->manager, i, &t)) {
                fprintf(stderr,
                        "monitor_manager_get_at: Cannot get child at %d\n", i);
                exit(1);
            }
            fds[i].fd = sockfds[i];
            fds[i].events = POLL_IN;
        }
    } else {
        // check for a specific process id so just 1 slot in <fds> array
        fds[0].fd = sockfd;
        fds[0].events = POLL_IN;
    }
}

// function to check all the fds and handle the messages from each one
static int check_fds(int bufsiz, void *_monitor, struct pollfd fds[], int nfd, int *active, MessageHandler handler, void *return_args[]) {
    TravelMonitor monitor = (TravelMonitor)_monitor;
    for (int i = 0; i < nfd; i++) {
        if (fds[i].fd == -1)continue;
        // the process closed the file
        if ((fds[i].revents & POLL_HUP) == POLL_HUP) {
            // process do not write anymore so just leave
            fds[i].fd = -1;
            fds[i].events = 0;
            fds[i].revents = 0;
            (*active)--;
            continue;
        }
        // we can read
        if ((fds[i].revents & POLL_IN) == POLL_IN) {
            // there's something to read
            char *msg = NULL;
            int len = 0;
            int opcode = -1;
            if (read_msg(fds[i].fd, bufsiz, &msg, &len, &opcode) == -1) return -1;

            // check if the process has stoped transmitting
            if (opcode == MSGEND_OP || opcode == SYN_OP || opcode == ACK_OP) {
                if (handler && (opcode == SYN_OP || opcode == ACK_OP))
                    handler(monitor, opcode, msg, len, return_args);
                if (len && msg) free(msg);
                // process do not write anymore so just leave
                fds[i].fd = -1;
                fds[i].events = 0;
                fds[i].revents = 0;
                (*active)--;
                continue;
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
    return 0;
}

int travel_monitor_get_response(int bufsiz, void *_monitor, MessageHandler handler, int sockfd, int *sock_fds, void *return_args[]) {
    // in travel monitor we don't care about <fd>

    // we will use poll to monitor the fifos
    // first set up the fds
    TravelMonitor monitor = (TravelMonitor)_monitor;
    int nfd = sockfd >= 0 ? 1 : monitor->num_monitors;
    struct pollfd fds[nfd];

    // initialize the fds array
    intialize_fds(monitor, nfd, fds, sockfd, sock_fds);
    // initialize the #monitors that are expected to send messages
    int active = nfd;
    while (active > 0) {
        int ret = poll(fds, nfd, 1);
        if (ret < 0) {
            // ignore signals
            if (errno == EINTR) continue;
            perror("poll in travel monitor");
            return -1;
        }
        if (ret) {
            if (check_fds(bufsiz, monitor, fds, nfd, &active, handler, return_args)==-1) return -1;
        }
    }
    return 0;
}

