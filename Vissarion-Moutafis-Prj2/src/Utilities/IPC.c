#include "IPC.h"

// Fifo I/O routines


// Protocol:
// msg = header|body
// header = opcode|size of message. (opcode: 1byte, size of msg: 10bytes)

// routine to create the header and attach it to the msg
static void set_header(char *msg, int body_len, int opcode) {
    // write opcode
    snprintf(msg, HDR_OP_LEN+1, "%c", opcode);
    // write msg body-length (padding of zeros in case the integer is less that the desired length)
    snprintf(msg + HDR_OP_LEN, HDR_MSGSIZE_LEN+1, "%0*d", HDR_MSGSIZE_LEN, body_len);
}

// function to send a message to fd
void send_msg(int fd, char *body, int body_len, int opcode) {
    // initialize a message buffer for the packet
    char msg[HDR_LEN + body_len];
    memset(msg, 0, HDR_LEN+body_len);
    // set the header of the packet
    set_header(msg, body_len, opcode);

    // check if there is a body
    if (body && body_len > 0) {
        // set the body of the packet
        char *body_part = msg+HDR_LEN;
        memcpy(body_part, body, body_len);
    }
    // send the packet into the fd
    if (write(fd, msg, HDR_LEN+body_len) == -1){perror("send_msg"); exit(1);}
}


// get the opcode from header
static int get_op(char *buffer) {
    return buffer[0];
}

// get the body length from header buffer
static int get_body_size(char *buffer) {
    char msg_len[HDR_MSGSIZE_LEN+1];
    memset(msg_len, 0, HDR_MSGSIZE_LEN+1);
    strncpy(msg_len, buffer+1, HDR_MSGSIZE_LEN);

    return atoi(msg_len);
}

// parse header
static void parse_header(char *buffer, int *body_len, int *opcode) {
    *opcode = get_op(buffer);
    *body_len = get_body_size(buffer);
}

// read the header
static void read_header(int fd, int bufsize, int *body_len, int *opcode) {
    char header[HDR_LEN];
    memset(header, 0, HDR_LEN);
    // first read the header
    if (my_read(fd, header, HDR_LEN, bufsize) == 0) {
        *body_len = 0;
        *opcode = -1;
    }
    // parse the header and assign values
    parse_header(header, body_len, opcode);
}

// function to read a message from fd
// We allocate memory for the body variable. User must free it. 
// NOTE THAT *body is exactly *body_len characters long, there is NO terminator
void read_msg(int fd, int bufsize, char **body, int *body_len, int *opcode) {
    // basic init of the returned variables
    *body = NULL;
    *body_len = 0;
    *opcode = -1;

    // first read the header from fd
    read_header(fd, bufsize, body_len, opcode);
    
    // check if the message has a body
    if (*body_len > 0) {
        // Now we have to read the actual body of the message
        *body = calloc(*body_len, sizeof(char));
        // read the body
        my_read(fd, *body, *body_len, bufsize);
    }
}

// Note that buffer must be assigned at least bytes_to_read bytes of memory
// classic read, wrapper, return 0 if EOF, else return 1
int my_read(int fd, char *buffer, int bytes_to_read, int bufsize) {
    if (bufsize > bytes_to_read)
        bufsize = bytes_to_read;

    int total_bytes_read = 0, bytes_read = 0;
    char buf[bufsize];
    memset(buf, 0, bufsize);

    while (total_bytes_read < bytes_to_read) {
        int bytes_left = bytes_to_read - total_bytes_read;
        int to_read = bytes_left < bufsize ? bytes_left : bufsize;

        // while we haven't read the whole message
        if ((bytes_read = read(fd, buf, to_read)) == -1) {perror("my_read"); exit(1);}
        if (bytes_read == 0) return 0;
        // insert the buf into the buffer, at proper place
        memcpy(buffer+total_bytes_read, buf, bytes_read);

        // increase the bytes_read counter
        total_bytes_read += bytes_read;

        // re-initialize the buf
        memset(buf, 0, bufsize);
    }
    return 1;
}