#pragma once

#include "BF.h"
#include "HT.h"
#include "List.h"
#include "SL.h"
#include "Types.h"
#include "Utilities.h"

// Declaration of the MonitorTrace struct.
typedef struct monitor_trace {
    pid_t pid;                  // PID of the monitor process instance, -1 if inactive
    int in_fifo;                // fifo file descriptors for input
    int out_fifo;               // fifo file descriptors for output
    char **countries_paths;     // The paths of the country dirs that are assigned to the monitor process 
    int num_countries;          // #countries assigned to the process
} MonitorTrace;

// Declaration of the MonitorManager
typedef struct monitor_manager {
    HT countries_index;         // key: country, item: monitor_trace pointer
    MonitorTrace *monitors;     // fixed array of monitors based on the pid of their process
    int num_monitors;           // size of the monitor manager
    int active_monitors;        // count of the truly active monitors
} *MonitorManager;

typedef struct trace {
    MonitorTrace *m_trace;
    char *country;
} *Trace;
// create monitor manager 
MonitorManager monitor_manager_create(int num_monitors);

// add a monitor into the manager
// return the index
int monitor_manager_add(MonitorManager manager, pid_t pid, int in_fifo, int out_fifo);

// search for monitor with pid='pid'. 'trace' copies that monitor trace if found. 
// Return index if found else -1
int monitor_manager_search_pid(MonitorManager manager, pid_t pid, MonitorTrace *trace);

// get monitor at i-th index. 'trace' copies that monitor trace if found.
// Return true if found else false
bool monitor_manager_get_at(MonitorManager manager, int i, MonitorTrace *trace);

// add a copy of 'country' to i-th monitor
void monitor_manager_add_country(MonitorManager manager, int i, char *country_path);

