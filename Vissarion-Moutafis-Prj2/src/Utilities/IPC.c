#include "IPC.h"

//routine to create fifos
void create_unique_fifo_pair(bool init, int unique_id, char *from, char *to) {
    if (init) {
        // check if the fifo dir exists. create it. return;
        if (access(FIFO_DIR, F_OK) == 0) delete_dir(FIFO_DIR);

        if (mkdir(FIFO_DIR, 0777) == -1) {
            perror("mkdir at fifo dir creation");
            exit(1);
        }
        return;
    }

    sprintf(to, "%s/to-monitor-%d.fifo", FIFO_DIR, unique_id);
    if (mkfifo(to, 0644) == -1) {perror("mkfifo"); exit(1);}
    sprintf(from, "%s/from-monitor-%d.fifo", FIFO_DIR, unique_id);
    if (mkfifo(from, 0644) == -1) {perror("mkfifo"); exit(1);}
}

void clean_fifos(void) {
    // check if the fifo dir exists. create it. return;
    if (access(FIFO_DIR, F_OK) == 0) delete_dir(FIFO_DIR);
    else {perror("clean fifos"); exit(1);}
}


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
    // initialize a message buffer
    char msg[HDR_LEN + body_len];
    memset(msg, 0, HDR_LEN+body_len);
    // set the header of the message
    set_header(msg, body_len, opcode);
    // set the body of the header
    char *body_part = msg+HDR_LEN;
    memcpy(body_part, body, body_len);

    // send the message into the fd struct
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
    my_read(fd, header, HDR_LEN, bufsize);
    // parse the header and assign values
    parse_header(header, body_len, opcode);
}

// function to read a message from fd
// We allocate memory for the body variable. User must free it. 
// NOTE THAT *body is exactly *body_len characters long, there is NO terminator
void read_msg(int fd, int bufsize, char **body, int *body_len, int *opcode) {
    // first read the header from fd
    read_header(fd, bufsize, body_len, opcode);

    // Now we have to read the actual body of the message
    *body = calloc(*body_len, sizeof(char));
    // read the body
    my_read(fd, *body, *body_len, bufsize);
}

// Note that buffer must be assigned at least bytes_to_read bytes of memory
// classic read, wrapper
void my_read(int fd, char *buffer, int bytes_to_read, int bufsize) {
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

        // insert the buf into the buffer, at proper place
        memcpy(buffer+total_bytes_read, buf, bytes_read);

        // increase the bytes_read counter
        total_bytes_read += bytes_read;

        // re-initialize the buf
        memset(buf, 0, bufsize);
    }
}