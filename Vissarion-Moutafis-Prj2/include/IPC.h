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
#define Q4_CHLD 7

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
void send_msg(int fd, char *body, int body_len, int opcode);

// function to read messages in fifos
// We allocate memory for the body variable. User must free it.
// NOTE THAT *body is exactly *body_len characters long, there is NO terminator
void read_msg(int fd, int bufsize, char **body, int *body_len, int *opcode);

// classic read wrapper. Return 1 in success and 0 when reads EOF
int my_read(int fd, char *buffer, int bytes_to_read, int bufsize);

////////////////////////////////////////////////////////////////////

// Basic non-blocking ipc for different parts of the program
// pass the monitor struct and 
// the process numerical id given from the monitor manager.
// Also pass the msg handler defined at IPC.h
// If <the process_id> is -1 then we wait for a response from all processes
// return > 0 in success else 0

// type
typedef int (*GetResponse)(void *monitor, MessageHandler handler, int process_id, int fd, void *return_args[]);

// actual response system
int travel_monitor_get_response(void *monitor, MessageHandler handler, int process_id, int fd, void *return_args[]);
int monitor_get_response(void *monitor, MessageHandler handler, int process_id, int fd, void *return_args[]);
