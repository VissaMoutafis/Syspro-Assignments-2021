/**
*	Syspro Project 2
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#include "IPC.h"
#include "Config.h"

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

static int special_write(int fd, char *buf, int bufsiz) {
    int bytes_wrote = write(fd, buf, bufsiz);
    if (bytes_wrote == -1) {
        if (errno == EINTR) return 0;
        return -1;
    }
    return bytes_wrote;
}

void my_write(int fd, char *msg, int bytes_to_write, u_int32_t bufsiz) {
    int total_written_bytes = 0;
    
    bufsiz = bufsiz > bytes_to_write ? bytes_to_write : bufsiz;
    char buffer[bufsiz];
    memset(buffer, 0, bufsiz);
    while (total_written_bytes < bytes_to_write) {
        // how many bytes left to write
        int diff = bytes_to_write - total_written_bytes;
        // how many bytes we will write in current iteration
        int to_write = bufsiz > diff ? diff : bufsiz;
        // copy them in the buffer
        memcpy(buffer, msg+total_written_bytes, to_write);
        // write the buffer
        int bytes_wrote = special_write(fd, buffer, to_write);
        if (bytes_wrote == 0)
            continue;
        else if (bytes_wrote == -1) {
            perror("send_msg");
            exit(1);
        }
        // update total_written_bytes
        total_written_bytes += to_write;
    }
}
// function to send a message to fd
void send_msg(int fd, u_int32_t bufsize, char *body, int body_len, int opcode) {
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
    my_write(fd, msg, HDR_LEN + body_len, bufsize);
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
static int read_header(int fd, u_int32_t bufsize, int *body_len, int *opcode) {
    char header[HDR_LEN];
    memset(header, 0, HDR_LEN);
    *body_len = 0;
    *opcode = -1;

    // first read the header
    if (my_read(fd, header, HDR_LEN, bufsize)==-1) return -1;

    // parse the header and assign values
    parse_header(header, body_len, opcode);

    return 0;
}

// function to read a message from fd
// We allocate memory for the body variable. User must free it. 
// NOTE THAT *body is exactly *body_len characters long, there is NO terminator
int read_msg(int fd, u_int32_t bufsize, char **body, int *body_len, int *opcode) {
    // basic init of the returned variables
    *body = NULL;
    *body_len = 0;
    *opcode = -1;

    // first read the header from fd
    if (read_header(fd, bufsize, body_len, opcode) == -1) return -1;
    
    // check if the message has a body
    if (*body_len > 0) {
        // Now we have to read the actual body of the message
        *body = calloc(*body_len, sizeof(char));
        // read the body
        if (my_read(fd, *body, *body_len, bufsize)==-1) return -1;
    }

    return 0;
}

// check if fd for read/write is open
// return 0 in success and -1 if fd has closed
int check_fd(int fd) {
    int all_ok = 0;
    struct pollfd fds[1];
    memset(fds, 0, sizeof(struct pollfd));
    fds[0].fd = fd;
    fds[0].events = POLL_IN | POLL_OUT;

    while (poll(fds, 1, 0) == -1 && errno == EINTR);
    if ((fds[0].revents & POLL_HUP) == POLL_HUP) all_ok = -1;

    return all_ok;
}

int special_read(int fd, char *buf, int to_read) {
    int bytes_read = read(fd, buf, to_read);
    if (bytes_read == -1) {
        // if sigchld occurs fail and go check everything, check your fd
        if (sigchld_set) return check_fd(fd);
        // ignore EAGAIN and EWOULDBLOCK
        if (errno == EAGAIN || errno == EWOULDBLOCK) return 0;
        // signal interruption: ignore signals (cannot escape SIGINT, SIGQUIT)
        if (errno == EINTR && !sigint_set && !sigquit_set) return 0;

        return -1;
    }

    return bytes_read;
}

// Note that buffer must be assigned at least bytes_to_read bytes of memory
// classic read, wrapper, return 0 if EOF, else return 1
// return 0 in success and -1 in error
int my_read(int fd, char *buffer, int bytes_to_read, u_int32_t bufsize) {
    if (bufsize > bytes_to_read)
        bufsize = bytes_to_read;

    int total_bytes_read = 0, bytes_read = 0;
    char buf[bufsize];
    memset(buf, 0, bufsize);

    while (total_bytes_read < bytes_to_read) {
        int bytes_left = bytes_to_read - total_bytes_read;
        int to_read = bytes_left < bufsize ? bytes_left : bufsize;
        
        // while we haven't read the whole message
        if ((bytes_read = special_read(fd, buf, to_read)) == -1) {
            char buf[100];
            sprintf(buf, "my_read %d, %d", getpid(), errno);
            perror(buf);
            return -1;
        }
        
        if (bytes_read == 0) continue;

        // insert the buf into the buffer, at proper place
        memcpy(buffer+total_bytes_read, buf, bytes_read);

        // increase the bytes_read counter
        total_bytes_read += bytes_read;

        // re-initialize the buf
        memset(buf, 0, bufsize);
    }
    return 0;
}

// Some Handlers of general purpose
// general handlers
void accept_syn(void *monitor, int opcode, char *msg, int msg_len, void *return_args[]) {
    // return_args[] = {bool is_syn}
    *((bool*)return_args[0]) = (opcode == SYN_OP);
}
void accept_ack(void *monitor, int opcode, char *msg, int msg_len, void *return_args[]) {
    // return_args[] = {bool is_ack}
    *((bool *)return_args[0]) = (opcode == ACK_OP);
}