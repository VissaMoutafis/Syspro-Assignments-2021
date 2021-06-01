/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#pragma once

#include "Types.h"
#include "HT.h"
#include "SL.h"
#include "List.h"
#include "BF.h"
#include "Utilities.h"
#include "FileUtilities.h"
#include "MonitorManager.h"
#include "IPC.h"
#include "Config.h"

// sl height for VirusStats::bf_per_countries
#define COUNTRY_SL_HEIGHT 10


// the declaration of the TravelMonitor type
typedef struct travel_monitor {
    int num_threads;             // number of threads in each one of the monitorServers
    int circular_buffer_size;   // size of the circular baffer 
    u_int32_t buffer_size;      // the size of the buffer for parent-child I/O via fifos
    int num_monitors;           // the number of generated monitorServers
    int accepted;               // accepted requests counter
    int rejected;               // rejected requests counter
    HT virus_stats;             // struct that keeps track of all bloom filters for the viruses
    MonitorManager manager;     // the monitor manager of the app
    size_t bloom_size;          // desired bloom_size   
} *TravelMonitor;


// Declaration of the VirusStats struct. This is the type 
// of items in the 'virus_stats' hashtable
typedef struct virus_stats {
    char *virus_name;           // The name of the virus 
    SL bf_per_countries;        // SL of BFs for the respective virus 
    List accepted;              // accepted requests with a date record
    List rejected;              // rejected requests with a date record
    List unique_bfs;            // helper struct in order to avoid double freeing when we kill the bfs
} *VirusStats;

typedef struct bf_tuple {
    char *country;
    BF bf;
} *BFTuple;

typedef struct request_record {
    char *date;
    char *countryTo;
} *RequestRec;


// init function
void travel_monitor_initialize(void);

// finalize func
void travel_monitor_finalize(TravelMonitor monitor);

// create a travel monitor
TravelMonitor travel_monitor_create(char *input_dir, size_t bloom_size, int num_monitors, u_int32_t buffer_size, int circular_buffer_size, int num_threads);

// destroy the travel monitor 
void travel_monitor_destroy(TravelMonitor monitor);

// basic workflow
bool travel_monitor_act(TravelMonitor monitor, int expr_index, char *value);

// basic children restore function
void travel_monitor_restore_children(TravelMonitor monitor);