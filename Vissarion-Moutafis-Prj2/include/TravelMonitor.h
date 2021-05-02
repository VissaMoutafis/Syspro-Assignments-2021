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
    u_int32_t buffer_size;      // the size of the buffer for parent-child I/O via fifos
    int num_monitors;           // the number of generated monitors
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
} *VirusStats;

typedef struct bf_tuple {
    char *country;
    BF bf;
} *BFTuple;

typedef struct request_record {
    char *date;

} *RequestRec;

TravelMonitor travel_monitor_create(char *input_dir, size_t bloom_size, int num_monitors, u_int32_t buffer_size);




// for init functionality: we call it at the creation of the monitor (inside the function)
// WARNING : DO NOT use outside of monitor module routines
bool initialization(TravelMonitor monitor, char *input_dir);

