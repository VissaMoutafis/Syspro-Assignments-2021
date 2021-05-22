/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#pragma once
#include "Config.h"
#include "Types.h"
#include "FileUtilities.h"
#include "TravelMonitor.h"

// Basic Log files setup

// create the log files
bool create_logs(void);


// Basic travel monitor set up

// for init functionality: we call it at the creation of the monitor (inside the
// function) WARNING : DO NOT use outside of monitor module routines
bool initialization(TravelMonitor monitor, char *input_dir);

// create a new monitor for the travel monitor
// or update the i-th monitor (for when the previous process holding it is dead)
bool create_monitor(TravelMonitor monitor, bool update, int i);
