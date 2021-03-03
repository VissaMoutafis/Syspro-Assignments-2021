#pragma once

#include <stdio.h>
#include "Types.h"
#include "HT.h"
#include "SL.h"
#include "BF.h"
#include "List.h"
#include "Utilities.h"

// as given by the instructors
#define BF_HASH_FUNC_COUNT 16

#define FIELD_SEPARATOR " "

// variable to determine whether the user wants to exit the application or not
bool is_end;

char error_msg[BUFSIZ];
bool error_flag;

// struct typedefs for the app
typedef struct vaccine_monitor *VaccineMonitor;

//  entry for the hash tables that contain the bloom filters
typedef struct virus_info_tuple {
    char *virusName;                // key of the tuple
    BF bf;                          // bloom filter to check if a person is not vaccinated
    SL vaccinated;                  // skip list with vaccinated people
    SL not_vaccinated;              // skip list with non-vaccinated people
} *VirusInfo;

typedef struct {
    Person p;
    char *date;
} * VaccRec;

typedef struct list_per_country {
    char *country;
    List citizen_list;
} *CountryIndex;

// Basic Vaccine Monitor methods

// initialize some basic variables that the monitor uses
void vaccine_monitor_initialize(void); 

// final changes to stop the program
void vaccine_monitor_finalize(void);

// Function to create a vaccine monitor 
VaccineMonitor vaccine_monitor_create(char *input_filename, int bloom_size, int sl_height, float sl_factor);

// function to destroy and deallocate the memory of a vaccine monitor
void vaccine_monitor_destroy(VaccineMonitor m);

// Basic functionality of the vaccine monitor. Return true if it succeeds.
// Returns false if it fail and an error message is saved in error_msg string
bool vaccine_monitor_act(VaccineMonitor monitor, int expr_index, char *value);


// helping functions
