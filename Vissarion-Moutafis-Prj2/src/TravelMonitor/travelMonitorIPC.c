#include "TravelMonitor.h"

static void intialize_fds(void *_monitor, int nfd, struct pollfd fds[], int process_id) {
    TravelMonitor monitor = (TravelMonitor)_monitor;
    
    if (process_id < 0) {
        // we set all the fd's in the manager monitor
        for (int i = 0; i < nfd; i++) {
            MonitorTrace t;
            memset(&t, 0, sizeof(Trace));
            if (!monitor_manager_get_at(monitor->manager, i, &t)) {
                fprintf(stderr,
                        "monitor_manager_get_at: Cannot get child at %d\n", i);
                exit(1);
            }
            fds[i].fd = t.in_fifo;
            fds[i].events = POLL_IN;
        }
    } else {
        // check for a specific process id so just 1 slot in <fds> array
        MonitorTrace t;
        memset(&t, 0, sizeof(Trace));
        if (!monitor_manager_get_at(monitor->manager, process_id, &t)) {
            fprintf(stderr, "monitor_manager_get_at: Cannot get child at %d\n", process_id);
            exit(1);
        }
        fds[0].fd = t.in_fifo;
        fds[0].events = POLL_IN;
    }
}

// function to check all the fds and handle the messages from each one
static void check_fds(int bufsiz, void *_monitor, struct pollfd fds[], int nfd, int *active, MessageHandler handler, void *return_args[]) {
    TravelMonitor monitor = (TravelMonitor)_monitor;
    for (int i = 0; i < nfd; i++) {
        // we can read
        if ((fds[i].revents & POLL_IN) == POLL_IN) {
            // there's something to read
            char *msg = NULL;
            int len = 0;
            int opcode = -1;
            read_msg(fds[0].fd, bufsiz, &msg, &len, &opcode);

            // check if the process has stoped transmitting
            if (opcode == MSGEND_OP) {
                // process do not write anymore so just leave
                (*active)--;
                continue;
            }
            // some handling
            handler(monitor, msg, len, return_args);
            // end of handling

            // free the memory of msg
            free(msg);
        }
    }
}

int travel_monitor_get_response(int bufsiz, void *_monitor, MessageHandler handler, int process_id, int fd, void *return_args[]) {
    // in travel monitor we don't care about <fd>

    // we will use poll to monitor the fifos
    // first set up the fds
    TravelMonitor monitor = (TravelMonitor)_monitor;
    int nfd = process_id >= 0 ? 1 : monitor->num_monitors;
    struct pollfd fds[nfd];

    // initialize the fds array
    intialize_fds(monitor, nfd, fds, process_id);
    // initialize the #monitors that are expected to send messages
    int active = nfd;
    while (active) {
        int ret = poll(fds, nfd, 1);
        if (ret) {
            check_fds(bufsiz, monitor, fds, nfd, &active, handler, return_args);
        }
        if (ret < 0){perror("poll"); exit(1);}
    }
    return 1;
}