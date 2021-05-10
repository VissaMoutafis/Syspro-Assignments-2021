#include "Setup.h"

int sigint_set = false;
int sigquit_set = false;
int sigusr1_set = false;

void monitor_sigint_handle(int s) { sigint_set = true; }

void monitor_sigquit_handle(int s) { sigquit_set = true; }

void monitor_sigusr1_handle(int s) { sigusr1_set = true; }

void monitor_signal_handlers(void) {
    struct sigaction sigint_act, sigquit_act, sigusr1_act;
    sigset_t set;
    memset(&sigint_act, 0, sizeof(struct sigaction));
    memset(&sigquit_act, 0, sizeof(struct sigaction));
    memset(&sigusr1_act, 0, sizeof(struct sigaction));
    memset(&set, 0, sizeof(sigset_t));

    if (sigfillset(&set) == -1) {
        perror("travel monitor sigset");
        exit(1);
    }
    // set the sigaction for sigint
    sigint_act.sa_mask = set;
    sigint_act.sa_handler = monitor_sigint_handle;
    if (sigaction(SIGINT, &sigint_act, NULL) == -1) {
        perror("sigint sigaction travel monitor");
        exit(1);
    }

    sigquit_act.sa_mask = set;
    sigquit_act.sa_handler = monitor_sigquit_handle;
    if (sigaction(SIGQUIT, &sigquit_act, NULL) == -1) {
        perror("siquit sigaction travel monitor");
        exit(1);
    }

    sigusr1_act.sa_mask = set;
    sigusr1_act.sa_handler = monitor_sigusr1_handle;
    if (sigaction(SIGUSR1, &sigusr1_act, NULL) == -1) {
        perror("sigusr1 sigaction travel monitor");
        exit(1);
    }
}