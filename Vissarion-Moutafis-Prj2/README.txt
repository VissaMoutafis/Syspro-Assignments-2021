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

The travel Monitor is the basic module that encloses the children-process generation the creation of fifos and logs
and the user query satisfaction. It is implemented as a wrapper that contains a hash table of <virusStats> that keep the blooms 
of each country group, lists of accepted/rejected requests with dates and a <MonitorManager> object that keeps the children 
monitoring easy and modular. 

Also we implement all the message handlers and IPC-related polling as described in later sections. 

The API is consisted of an init, a create, an act, a destroy and a finalize routines, that keep the modularity of the struct, 
while we also provide a generic handler for SIGCHLD that will check for dead-children and will restore them by generating 
new ones (check in the respective section of signal handling).

The main functions starts with init routines (send init stats, init and create travelMonitor object, wait for bloom filters)
and then moves to a loop that will end only if the user give the command or a SIGINT/SIGQUIT occurs.

The loop uses the TTY API to print and get the message and by setting some globals, we inform the TTY what the accepted commands are, 
what the number of their args are and other superficial error checking details.

After the travel monitor get the parsed command it proceeds in a switch that will decide what function to choose.

All of the queries are implemented based on the assignment's reading.

At exit we print logs, kill children and proceed to wait all children finish and finally free memory and exit.


## Monitor Details ##

The monitors are implemented in the same spirit as the vaccineMonitors of the first excercise. Specificaly I only expanded the struct declaration
and changed the query functionalities to suit the assignments specifications. Some extra changes there are in the main loop, since monitors 
do not wait for stdin input but for fifo input. We implement this by manifesting a <get_response> function that will perfom a poll operation. 
Check next section for that.

The init routine is the first part of the main function:
- get bloom size and buffer size,
- get directories,
- init a FM (file manager module) to have a generic directory I/O struct, given the dirs, 
- init a (vaccine) monitor wrapper, using all the appropriate stats,
- send the bloom filters to parent and
- wait for a SYN packet and send back an ACK in order to check availability.

After that we enter a loop that ends only if the parent process sends a kill signal, or a SIGINT\SIGQUIT signal is send to the process. 
In each iteration we wait for a query from the server and we answer based on the same <monitor_act> routine we manifested in the first assignment. 

The only special case is when a SIGUSR1 is handled where the monitor breaks from poll (check signal handling and IPC sections),
detects that the expr_index and value are NULL and checks for sigusr1, that whether it is set true, we use <monitor_act> in a 
very specific way so that we check dirs for new files and then resend bloom filters to the travelMonitor.

Finaly the logs are printed every time the process gets SIGINT\SIGQUIT and uses all the stats it has gathered through out its life.


## IPC w\ fifos ##

The fifo was the biggest part of IPC related activities and to make that re-usable and easy to use we manifested
wrappers for the low-level I/O calls read and write. In these calls we pass the <buffer> we want to read/write to/from, the 
<bufsiz> of the buffer, the <fd> that we will perform the read/write  and the <bufferSize> that manifests
the max ammount of bytes we read/write in each I/O syscall. We also changed the default return values of the I/O syscalls
via the wrappers, in order to suit the signal handling behaviour we want them to attain and the overall 
I/O behaviour in a non-blocking manner.

-> I/O wrappers:

We followed the following protocol. We send packets through the fifos, that have the following 
format:
<1-byte operation code><0-padded byte-size of body (10 bytes total)><actual body of arbitary size>

- the constant header that every packet had is consisted of the operation code and the body length
in order to enable null-body packet sending for symbolic messages via the processes. i.e. when a process stops message transmission
for the current undergoing functionality (for example initiazation) we send a header with MSGEND_OP, 
body size 0 (10 bytes all zeros) and there is no body.
- the body is what the user actualy has access to and it can be anything he/she likes (even another nested packet)

In order to keep the protocol robust, we follow the following I/O logic:
1. write/read HDR_LEN bytes, <bufSize> per I/O, in order to communicate the whole header and
    - (read) learn how many bytes we gotta read before returning
    - (write) make sure the header is the first thing that is passed in the fifo 
2. Read/Write the body:
    - (read) first get the body_size from the header we just read and then get the body, <bufSize> bytes per I/O. 
    Then pass it to the appropriate buffer for return and inform the user for the body size       
    - (write) write, <bufSize> bytes per I/O, the whole body according to the size the user gave us.
3. After reading/writting the whole packet return the respective info to caller.

SPECIAL CASE: there is a check for empty body that implies the sender wished 
to send only a header, with a symbolic meaning.

In most of cases when a process finish transmitting packets for a functinality/query/init routine it sends a MSGEND_OP packet (header-only),
to show that the transmission has ended.

-> Polling:
In order to detect I/O in an edge-case manner we use poll. We provide 1-sole polling API, the <get_response> like functions.
These are defined are one type but there are 2 different implementations each one, processes-type specific (1 for parent, 1 for children).
We also provide message handlers that will actualy process the message-body and parse it accordingly, based on a given protocol (check later).
Finaly we pass a void *[] array as return_args in case any of the signal handlers has to return stuff.

-> Protocols:
-monitor receiving init stats: msg-body = <buffer size (10 bytes long)><bloom size (10 bytes long)>
-monitor receiving directories: msg-body = <country>$<country>$...etc
-monitor receiving queries: msg-body = <command index that reselmbles a command (10-bytes)><command args of arbitary length>
-travelMonitor receiving bloom filter from child: msg-body = <num of bytes till bloom filter nested-packet (10 bytes)><virusName>$<c1><c2>...<bloom filter packet>
NOTE that we send one bloom at a time and we include the neccessary info at the start of the msg-body. We use this
protocol since writting huge amounts of data will delay the polling since the I/O is perfomed over a whole packet 
and not just the bytes that are currently in the fifo.
-travelMonitor receiving travelRequest answer: <YES/NO> <date of vaccination if ans==YES>
-travelMonitor receiving vaccination Status from child: <answer formated by the child>
-bloom filter packet: <10 bytes of size><BF's bitstring>

The get_response and message handler routines are all implemented in relative files inside the travelMonitor & monitor modules.


## Signal Handling Details:
flags, init-setup, I/O + poll syscalls modifications, KILL -> no handler, SIGINT|SIGTERM -> free memory, send kill to children => not all children print logs

We set up some signal handler's specificaly for each process and we configure them after cmd argument checking in 
main of each process, as part of initialization routine.

We specifically implemented read/write wrappers that will ignore the signals (checking the value of errno) and will return 0 instead of -1
in that way we will actualy ignore the error code. Case that we don't want that is that a SIGINT/SIGQUIT is send, so the call will actualy fail.

After SIGINT/SIGTERM is send to all monitors + travelMonitor (ctrl + C or ctrl + \) everyone is starting to quit.
So inevitably monitors that has not exited yet, will receive KILL and die before print out logs, or while they are printing them.
This is something that cannot be handled since the SIGKILL cannot be handled.
Anyways if we just send SIGINT/SIGTERM to one monitor at a time, everything work out well.

The SIGCHLD signal is async-handled by the travelMonitor as follows:
- iterate through the MonitorManager's process pid array,
- use waitpid for specific pid with WNOHANG option enabled,
- if the waitpid returns the PID of the child, then we restore it (init + send/receive blooms etc), else we continue
- if in the meantime another SIGCHLD has occured then handle the current monitor, reinit the check and start over
- return to the main loop

The SIGCHLD is also ignored since that we will not hang in polling because we keep track of active monitors and this counter
decreases in case of reading a MSGEND_OP header OR in case fds[i].revents == POLHUP so that we make sure we will not wait for 
a dead monitor and hang forever.

When a SIGCHLD occurs during reading a packet we check the fd with a poll syscall to check whether the
process writting on it is still running and connected (otherwise revents == POLL_HUP) and continue.
if it does not we just fail the query and restore dead children.
The user will have to retype the query so he get an answer.   

## Utilities Details:

Inside the utilities module we contain all the utility functions of the previous assignment + functions for argument parsing 
that is commonly used ammong the parent and children processes. Also we added IPC.c that contains the generic wrappers and some 
standardized packet handlers (SYN/ACK handlers). Rest of code inside the utilities consists the sigaction set up 
and the struct manipulation functions that were extended to accomodate the second assignment's structs. 
