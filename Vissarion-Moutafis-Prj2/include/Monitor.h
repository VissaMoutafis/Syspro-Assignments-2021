#pragma once

#include "Types.h"
#include "HT.h"
#include "SL.h"
#include "BF.h"
#include "List.h"
#include "Utilities.h"
#include "FM.h"
#include "IPC.h"

// as given by the instructors
#define BF_HASH_FUNC_COUNT 16

#define FIELD_SEPARATOR " "

// variable to determine whether the user wants to exit the application or not
bool is_end;

char error_msg[BUFSIZ];
bool error_flag;

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
    int sl_height;
    float sl_factor;
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
Monitor monitor_create(FM fm, int bloom_size, int sl_height, float sl_factor);

// function to destroy and deallocate the memory of a vaccine monitor
void monitor_destroy(Monitor m);

// Basic functionality of the vaccine monitor. Return true if it succeeds.
// Returns false if it fail and an error message is saved in error_msg string
bool monitor_act(Monitor monitor, int expr_index, char *value);


