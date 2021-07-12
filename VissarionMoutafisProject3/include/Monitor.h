/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#pragma once

#include "Types.h"
#include "HT.h"
#include "SL.h"
#include "BF.h"
#include "List.h"
#include "Utilities.h"
#include "FM.h"
#include "CB.h"
#include "IPC.h"
#include "Config.h"


// struct typedefs for the app
typedef struct monitor {
    // Prj 2 extension: A file manager that keeps track of some files. 
    // It is supplied by the user
    FM fm;
    // Prj 2 extension: Accepted/Rejected requests
    int accepted;
    int rejected;
    // General indexing for citizens
    HT citizens;
    List citizen_lists_per_country;
    // hash table {key: virusName, items:[bf, vacc_sl, non_vacc_sl]}
    List virus_info;
    int bloom_size;
    int buffer_size;
    int sl_height;
    float sl_factor;
    // extensions of 3rd assignment
    CB cb;
} * Monitor;

//  entry for the hash tables that contain the bloom filters
typedef struct virus_info_tuple {
    char *virusName;                // key of the tuple
    BF bf;                          // bloom filter to check if a person is not vaccinated
    SL vaccinated;                  // skip list with vaccinated people
    SL not_vaccinated;              // skip list with non-vaccinated people
} *VirusInfo;

typedef struct vaccine_record{
    Person p;
    char *date;
} * VaccRec;

typedef struct list_per_country {
    char *country;
    List citizen_list;
} *CountryIndex;

// Basic Vaccine Monitor methods

// initialize some basic variables that the monitor uses
void monitor_initialize(void); 

// final changes to stop the program
void monitor_finalize(Monitor monitor);

// Function to create a vaccine monitor 
Monitor monitor_create(FM fm, int bloom_size, int buffer_size, int circular_buffer_size, int sl_height, float sl_factor);

// function to destroy and deallocate the memory of a vaccine monitor
void monitor_destroy(Monitor m);

// Basic functionality of the vaccine monitor. Return true if it succeeds.
// Returns false if it fail and an error message is saved in error_msg string
bool monitor_act(Monitor monitor, int expr_index, char *value);


// EXTENSIONS FOR IPC HANDLING

// sending the bloom filters as a whole message 
// all divided by a SEP character
// header of the packet has INIT_PAR
void monitor_send_blooms(Monitor monitor, int to_fd);

// function to insert a record into the monitor 
// if we want to update an already inserted records then set update==true
void insert_record(Monitor monitor, char *record, bool update);

// routine that every thread follows
void *monitor_thread_routine(void *_args);

// routine for the main thread (producer)
void monitor_producer_routine(Monitor monitor, char **files, int flen);

// clean up threads
void clean_up_threads(Monitor monitor);