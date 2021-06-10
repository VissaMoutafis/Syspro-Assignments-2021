* * * SYSPRO Project 3 - Vissarion Moutafis (sdi1800119)* * *

//////////////////////
Abstract                
//////////////////////

The third project was essentially the same as the second one but with the following technical differences:
- The IPC was over sockets, not named-pipes
- There was no use of signals
- The queries that used signals, are implemented as the rest of them - send query to server over socket and wait for response.
- The init process changed a little since the 2 programs arguments changed
- Each monitor process had its own threads to parse and insert records from files in a distributed manner.
- There was a need for mutexes and conditional variables, due to the previous modification.


In the following paragraphs I will provide you with all necessary implementation details you need to know,
nevertheless I will not get to informative since the functionalities' code is basically the same as in the previous assignment. 
I will elaborate the network protocols, the changed-queries implementation and the init protocol,
as well as the threading protocol, used for distributed directory management.  


//////////////////////
Compilation and Run 
//////////////////////

~$ make [all] [DEBUG=1]         # compile the program
~$ make run [DEBUG=1]           # compile and run with standard arguments (check the Makefile if you wanna change the default arguments)
~$ make rerun [DEBUG=1]         # re-compile and run with default arguments
~$ make valgrind-run [DEBUG=1]  # compile and run with default argument using the valgrind utility *
~$ make clean                   # clean all objective and executables
~$ make clean-logs              # clean all log files 

* If you wanna run this then make sure you installed valgrind: 
    ~$ sudo apt install valgrind # install valgrind


//////////////////////
Networking Protocols  
//////////////////////
We used the same I/O protocol with packets as in assignment 2. We also used the write, read syscalls since there were all-set
from the previous assignment.

The sockets that are binded will be modified to allow port reusal after they are closed, 
for easier use of ports after program termination.

We provide a very specific port range to the app. Check Networking.h

We used the following CLIENT protocol:
- 1 client that we'll connect to <n> servers. At fork stage assign a unique port to child/server, 
    known to client. We need to save them all in a management struct.

- Every time the client wants to communicate with a server he will estasblish a connection with him using a loop of type
    do {
        wrtfd = connect(ip_addr, port) // try connect
    } while(wrtfd is not set) // keep trying till we succeed

    and after that he will use this socket for bidirectional communication. The socket will be writeable AND readable,
    so it will resemble both in and out fifo of the 2nd assignment. There will also be the same fd-readability check
    with the same poll-based utility function I provided in the second assignment. 

- At the init state we will first assign the dirs and fork the children/servers, after that we will estasblish a connection to all of them.
    Lastly, we will start polling and get all the blooms in packets. The packet handler for blooms is the same as in 2nd assignment.

- We will be sending all queries via sockets and then start polling of the respective socket(s) fd(s) for the answer.

- At the end of the program, instead of a SIGKILL as in 2nd assignment we just send a special termination packet (like a query packet) 
    to all the forked servers and after that we will wait for all of them to exit so that the main process/client exits as well.


We used the following SERVER protocol:
- We will create a single socket for the sole reason of accepting connections and assign them different socket-fds

- We will use the <wait_connection> API call that was especially crafted to accept 
    connections and print the appropriate error messages if needed.

- At the init state we will set up the server with the networking stats the server received at exec and after that 
    we will set up the blooms filters, wait for client to connect and send them over a socket.

- After that we will establish the main loop that server uses to respond client requests. We will wait for a connection, accept it,
    parse the query and send the respective answer back, or terminate (if query is the "exit" command). This simple strategy ensures that
    the servers use the least possible CPU cycles.

- At the end we will terminate and close all the respective connections. 


Note: The one connection per request protocol is used so that there are no more fd open than needed. 
Also the server is implemented to act, only when a connection - and a following request -  occurs.
This protocol ensures that servers won't be wasting CPU circles since the <accept> syscall blocks till a connection occurs.

Also during the init, there is a stage where client waits for a fake-SYN packet and sends an fake-ACK packet,
so that packets are just send for the sake of checking that everything works after init process.


//////////////////////
Exec & Init Changes                
//////////////////////
The main changes in the parent program is the fact that the directories cannot be send. They MUST be passed at the time of 
fork during the exec call the child proceeds to. For that reason we will modify the Process Manager we used in the 2nd assignment 
so that we can set up the directories before the fork takes place.
After that, passing the parameters is easy.

The same goes for stats since they're also part of command line args of the exec call. So the parent after forking will only connect to the servers-children
and wait for the blooms.

Further init changes in the monitor-servers are consisted of the thread management and will be described later.


//////////////////////
Query Changes                
//////////////////////

The only thing we changed in the query handling is that now the "addVaccinationRecords" query string from client, as well as the answer packet,
are passed over the communication socket. The same goes for the exit command. 


//////////////////////
Threading                
//////////////////////

We used threads to distributed the workload of passing mutliple directories' record  into the monitor servers 
database structs. To do something like that we established the following monitor side algorithm, that resembles the producer-consumer algorithm for multiple consumers.


>>>>>>>>>>>>>>> GLOBALS

vars :: is_end, monitor, rec, file_paths 
mutex :: cb_empty_m, cb_full_m, monitor_m, cb_m
condition :: cb_empty_c, cb_full_c

>>>>>>>>>>>>>>> MAIN THREAD-Producer ROUTINE:
files = read_file_paths

for file_path in files:
    lock(cb_full_m)

    while (cb_is_full)
        wait(cb_full_c)
    
    unlock(cb_full_m)

    lock(cb_m)
    add file into the circular buffer
    unlock(cb_m)

    lock(cb_empty_m)
    signal(cb_empty_c)
    unlock(cb_empty_c)

AT_TERMINATION:
    is_end = true;
    while (!all_threads_done) {
        broadcast(cb_empty_c)
    }

    clean-up



>>>>>>>>>>>>>>> THREADS-Consumers ROUTINE:
while (!is_end) {
    lock(cb_empty_m)

    while (cb_empty AND !is_end)
        wait(cb_empty_c)
    
    unlock(cb_empty_m)

    if (!is_end) {
        lock(cb_m)
        read file path from circular buffer
        unlock(cb_m)

        lock(cb_full_m)
        signal(cb_full_c)
        unlock(cb_full_m)

        if (we did read a new file) {
            for rec in records:
                lock(monitor_m)
                insert(rec)
                unlock(monitor_m)
        }    
    }
}


Notes: 

    We make sure that the producer changes all the variables and add data in the CB ATOMICALLY with the mutexes. ALSO we make sure that 
every time it adds stuff it will signal the empty cond so that one consumer waiting on it will unblock and proceed its processing. Finally we make sure
that the producer will not try to add stuf to a FULL CB since we always block on a condition variable called cb_full_c.

    Consumer side, we make sure that only one consumer will actualy get into the processing per data produced, since all threads are waiting on the 
cb_empty_c condition. After that we make some checks for exit case and proceed to remove 1 file path and add all of its records (each one atomically)
into the monitor database. After that we signal the cb_full_c condition so that the master thread knows that at least one CB slot is free. 
Note that the consumers "sleep" on the condition variable, while there are no new files added into the circular buffer. 

AT THE END we will change the variable and broadcast the condition since this is the part that every thread will be waiting on.

