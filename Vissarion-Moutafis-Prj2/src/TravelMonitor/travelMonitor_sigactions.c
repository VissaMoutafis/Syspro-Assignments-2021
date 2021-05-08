#include "Setup.h"

int sigint_set = false;
int sigquit_set = false;
int sigchld_set = false;

void travel_monitor_sigint_handle(int s) {
    sigint_set = true;
}

void travel_monitor_sigquit_handle(int s) {
    sigquit_set = true;
}

void travel_monitor_sigchld_handle(int s) {
    sigchld_set = true;
}


void travel_monitor_signal_handlers(void) {
    struct sigaction sigint_act, sigquit_act, sigchld_act;
    sigset_t set;
    memset(&sigint_act, 0, sizeof(struct sigaction));
    memset(&sigchld_act, 0, sizeof(struct sigaction));
    memset(&sigquit_act, 0, sizeof(struct sigaction));
    memset(&set, 0, sizeof(sigset_t));

    if (sigfillset(&set) == -1){perror("travel monitor sigset"); exit(1);}
    // set the sigaction for sigint
    sigint_act.sa_mask = set;
    sigint_act.sa_handler=travel_monitor_sigint_handle;
    sigint_act.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sigint_act, NULL) == -1) {perror("sigint sigaction travel monitor"); exit(1);}

    sigquit_act.sa_mask = set;
    sigquit_act.sa_handler = travel_monitor_sigquit_handle;
    sigquit_act.sa_flags = SA_RESTART;
    if (sigaction(SIGQUIT, &sigquit_act, NULL) == -1) {
        perror("siquit sigaction travel monitor");
        exit(1);
    }

    sigchld_act.sa_mask = set;
    sigchld_act.sa_handler = travel_monitor_sigchld_handle;
    sigchld_act.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sigchld_act, NULL) == -1) {
        perror("sigchld sigaction travel monitor");
        exit(1);
    }
}