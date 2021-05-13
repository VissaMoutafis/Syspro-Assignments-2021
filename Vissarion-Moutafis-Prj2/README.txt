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
The fifo was the biggest part of IPC related activities and to make that re-usable and easy to use we manifested
wrappers for the low-level I/O calls read and write. In these calls we pass the <buffer> we want to read/write to/from, the 
<bufsiz> of the buffer, the <fd> that we will perform the read/write  and the <bufferSize> that manifests
the max ammount of bytes we read/write in each I/O syscall. We also changed the default return values of the calls
via the wrappers in order to suit the signal handling behaviour we want them to attain and the overall 
read behaviour in a non-blocking manner.

We followed the following protocol. We send packets through the fifos, that have the following 
format:
<1-byte operation code><0-padded body byte-size (10 bytes total)><actual body of arbitary size>

- the constant header that every packet had is consisted of the operation code and the body length
in order to enable null-body packet sending for symbolic messages via the processes. i.e. when a process stops message transmission
for the current undergoing functionality (for example initiazation) we send a header with MSGEND_OP, 
body size 0 (10 bytes all zeros) and there is no body.
- the body is what the user actualy uses and it can be   

## Signal Handling Details:


## Utilities Details:


