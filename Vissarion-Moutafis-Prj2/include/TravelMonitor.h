#pragma once

#include "Types.h"
#include "HT.h"
#include "SL.h"
#include "List.h"
#include "BF.h"
#include "Utilities.h"

// the declaration of the TravelMonitor type
typedef struct travel_monitor {
    int accepted;               // accepted requests counter
    int rejected;               // rejected requests counter
    HT virus_stats;             // struct that keeps track of all bloom filters for the viruses
    int bloom_size;             // desired bloom_size   
} *TravelMonitor;


// Declaration of the VirusStats struct. This is the type 
// of items in the 'virus_stats' hashtable
typedef struct virus_stats {
    char *virus_name;           // The name of the virus 
    SL monitor_blooms;          // SL containing the bloom filters and some more details
    List accepted;              // accepted requests with a date record
    List rejected;              // rejected requests with a date record
} *VirusStats;

// Declaration of the MonitorTrace struct.
typedef struct monitor_trace {
    pid_t pid;                  // PID of the monitor process instance 
    char *fifos[2];             // fifos assigned to the monitor for communication
    char **countries_paths;     // The paths of the country dirs that are assigned to the monitor process 
} *MonitorTrace;

// Declaration of the MonitorBloomEntry struct. item of 'monitor_blooms' skip list
typedef struct monitor_blooms_entry {
    MonitorTrace monitor;
    BF bf;
} *MonitorBloomEntry;

