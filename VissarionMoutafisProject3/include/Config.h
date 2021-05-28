/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#pragma once
#include "Types.h"
#include "Networking.h"

// def bufsiz
#define DEF_BUFSIZ 32

// NETWORKING CONFIG for app
#define CLIENT_PORT NET_LOWEST_PORT
#define SERVER_LOWEST_PORT (CLIENT_PORT+1)

// for logging
#define ROOT_LOG_PATH "./logs"
#define MONITOR_LOG_PATH "./logs/monitors"
#define TRAVEL_MONITOR_LOG_PATH "./logs/travel-monitor"

// defaults, travel_monitor specific
#define DEF_NUM_MONITORS 1
#define DEF_BLOOM_SIZE 100
#define DEF_BUFFER_SIZE 100
#define DEF_CBUF_SIZE 3
#define DEF_NUM_THREADS 2

// defaults, monitor specific
#define SL_HEIGHT 10
#define SL_FACTOR 0.5


// general defaults
#define BF_HASH_FUNC_COUNT 16       // number of hash functions of a bloom filter

#define FIELD_SEPARATOR " "         // how the cmd command's value-fields are separeted

// variable to determine whether the user wants to exit the application or not
bool is_end;

char error_msg[BUFSIZ];
bool error_flag;

// these are specific variables that we will set for use in server and client apps

in_addr_t _ip_addr_; // ip address that we will send/receive messages
int _port_;   // port that the app will be listening
int listener_sockfd;    // socket that the servers will use in order to accept connections
int connection_sockfd;  // general-use socket for client-server communication 


// include only monitor code
#include "Threading.h"
ThreadArgs thread_args;
bool update_at_insert;          // a global configuration that helps with monitor insertions
bool thread_end;                // variable that signals the end of the thread execution
int threads_done;
int num_threads;
pthread_t *threads;

// mutexes for resources
pthread_mutex_t mtx_cb;         // mutex that locks every time a thread get a file from cycle buffer
pthread_mutex_t mtx_monitor;    // mutex that locks when we apply a change into the Monitor struct
pthread_mutex_t mtx_check_end;  // mutex that locks everytime we check for end of execution

// conditionals and their mutexes
pthread_cond_t cond_cb_full;
pthread_mutex_t mtx_cb_full;
pthread_cond_t cond_cb_empty;
pthread_mutex_t mtx_cb_empty;
