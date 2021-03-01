#pragma once

#include <stdio.h>
#include "Types.h"
#include "BF.h"
#include "HT.h"
#include "SL.h"
#include "Utilities.h"

// as given by the instructors
#define BF_HASH_FUNC_COUNT 16



// variable to determine whether the user wants to exit the application or not
bool is_end;

char *error_msg;

// struct typedefs for the app
typedef struct vaccine_monitor *VaccineMonitor;


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
