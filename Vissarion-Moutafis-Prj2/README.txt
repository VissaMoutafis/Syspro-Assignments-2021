Syspro Project 2 Documentation - Written By Vissarion Moutafis 

AM : sdi1800119
Name : Vissarion Moutafis


Abstract:
    The project is consisted of 6 main parts:
        - The Monitor module, that is used for the worker behaviour.
        - The TravelMonitor module, that is the main process, 
        which controls everything and is the end the user interacts with.
        - The data structure modules, which are helping structs for the proper structure of the upper modules.
        - The IPC with fifos module, that contains generic wrappers for <read>, <write> and <poll> system calls.
        Check Utility functions for that module.
        - The signal handling module, that is solely setting the signal handlers for each process at the beginning
        of its run.
        - General Utilities (that were used in the previous PRJ), with some extra extensions for TTY API, string manipulation
        struct manipulation (compare, destroy etc.), argument checking, file utility routines and others. This module also
        contains the generic read, write wrappers.


Make Options:
We used 1 general makefile and 2 .mk files for better organized compilation.

~$ make [all] [DEBUG=1]# make every-component. w\ DEBUG=1 we can check for debbuging prints and assertions
~$ make run # make all and run with default args (check Makefile)
~$ make clean # clean all binaries and objective files
~$ make re-run # re-compile and run
~$ make clean-logs # clean the log-files and the log dir



## Travel Monitor Details:


## Monitor Details:


## IPC w\ fifos Details:


## Signal Handling Details:


## Utilities Details:


