#pragma once
#include "Config.h"
#include "Types.h"
#include "FileUtilities.h"
#include "TravelMonitor.h"

// Basic Fifo Setup

// create a unique fifo. if init is true create only the fifo directory
void create_unique_fifo_pair(bool init, char *from, char *to);

// routine to clean up the fifos
void clean_fifos(void);

// Basic Log files setup

// create the log files
bool create_logs(void);


// Basic travel monitor set up

// for init functionality: we call it at the creation of the monitor (inside the
// function) WARNING : DO NOT use outside of monitor module routines
bool initialization(TravelMonitor monitor, char *input_dir);


// Signal Handling configurations
int sigint_set;
int sigquit_set;
int sigchld_set;
int sigusr1_set;

void travel_monitor_signal_handlers(void);
void monitor_signal_handlers(void);