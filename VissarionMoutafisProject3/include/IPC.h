/**
*	Syspro Project 2
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#pragma once

#include "Types.h"
#include "MessageHandlers.h"

// Protocol:
// msg = header|body
// header = opcode|size of message. (opcode: 1byte, size of msg: 10bytes)

#define HDR_LEN 11             // the byte-length of msg header
#define HDR_OP_LEN 1           // the byte-length of header's opcode
#define HDR_MSGSIZE_LEN 10     // the byte-length of header's msg-size field
#define SEP "$"                // separator if needed to divide data

// AT THE END OF TRANSMISSION ALWAYS SEND A MSGEN_OP HEADER (with empty body if you like)

////////////////////////////////////////////////////////

// Parent-process specific opcodes (THESE ARE RETURNED TO PAR)

#define INIT_PAR 0
#define Q1_PAR 2
#define Q4_PAR 4

// Child-process specific opcodes (THESE ARE SENT TO CHLD)

#define INIT_CHLD 1         // for initialization stuff
#define COUNTRY_CHLD 3      // for country path sending
#define Q1_CHLD 5           // for query 1 
#define Q3_CHLD 7
#define Q4_CHLD 11

// general opcodes
#define MSGEND_OP 20
#define SYN_OP 21
#define ACK_OP 22
#define EXIT_OP 23

// return codes
#define OK_RET 30

///////////////////////////////////////////////////////////////////

// I/O Routines

// function to send messages in fifos 
void send_msg(int fd, u_int32_t bufsize, char *body, int body_len, int opcode);

// function to write a msg of len <bytes_to_write> in fd, with a buffer of size <bufsiz>
void my_write(int fd, char *msg, int bytes_to_write, u_int32_t bufsiz);

// function to read messages in fifos
// We allocate memory for the body variable. User must free it.
// NOTE THAT *body is exactly *body_len characters long, there is NO terminator
int read_msg(int fd, u_int32_t bufsize, char **body, int *body_len, int *opcode);

// classic read wrapper.
// return 0 in success and -1 in error (print a perror message in error)
int my_read(int fd, char *buffer, int bytes_to_read, u_int32_t bufsize);

////////////////////////////////////////////////////////////////////

// Basic non-blocking polling of socket fd's (alternative of previous <get response> for fifos)
// bufsiz is the max length we can read (given by the user) 
// pass the monitor struct (either TravelMonitor or Monitor)
// Also pass the msg handler defined at IPC.h
// If <sockfd> is -1 then we wait for a response from all processes, TravelMonitorClient specific.
// <sockfd> is specificly used to determine the input socket fd, if it's greater than 0. 
// return 0 in success else -1
// type
typedef int (*GetResponse)(int bufsiz, void *monitor, MessageHandler handler, int sockfd, int *sock_fds, void *return_args[]);

// actual response system

// for travel monitor (parent process)
int travel_monitor_get_response(int bufsiz, void *monitor, MessageHandler handler, int sockfd, int*sock_fds, void *return_args[]);

// for monitors (child process)
int monitor_get_response(int bufsiz, void *monitor, MessageHandler handler, int sockfd, int *sock_fds, void *return_args[]);
