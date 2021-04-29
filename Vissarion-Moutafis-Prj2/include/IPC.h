#pragma once

#include "FileUtilities.h"
#include "Types.h"

#define FIFO_DIR "fifos"
#define DEF_BUFFER_SIZE 100

// Protocol:
// msg = header|body
// header = opcode|size of message. (opcode: 1byte, size of msg: 10bytes)

#define HDR_LEN 11             // the byte-length of msg header
#define HDR_OP_LEN 1           // the byte-length of header's opcode
#define HDR_MSGSIZE_LEN 10     // the byte-length of header's msg-size field

// operational codes for the protocol
// opcodes % 2 == 0 : parent -> child
// opcodes % 2 == 1 : child -> parent
// some opcodes are general purpose (specified below)
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
#define SYN_OP 30
#define ACK_OP 31
#define EXIT_OP 70


///////////////////////////////////////////////////////////////////

// Basic Fifo Setup

// create a unique fifo. if init is true create only the fifo directory
void create_unique_fifo_pair(bool init, int unique_id, char *from, char *to);

// routine to clean up the fifos
void clean_fifos(void);

// I/O Routines

// function to send messages in fifos 
void send_msg(int fd, char *body, int body_len, int opcode);

// function to read messages in fifos
// We allocate memory for the body variable. User must free it.
// NOTE THAT *body is exactly *body_len characters long, there is NO terminator
void read_msg(int fd, int bufsize, char **body, int *body_len, int *opcode);

// classic read wrapper
void my_read(int fd, char *buffer, int bytes_to_read, int bufsize);