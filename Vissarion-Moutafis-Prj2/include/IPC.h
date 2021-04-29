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

// init 
#define COUNTRIES_OP_PARENT 0
#define COUNTRIES_OP_CHILD  1

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