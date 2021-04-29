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

// Protocol:
// msg = header|body
// header = opcode|size of message. (opcode: 1byte, size of msg: 10bytes)

// function to send a message to fd
void send_msg(int fd, char *body, int body_len, int opcode) {
    char header[HDR_LEN+1];
    memset(header, 0, HDR_LEN+1);
}

// function to read a message from fd

int get_op(char *buffer) {
    return buffer[0];
}

int get_body_size(char *buffer) {
    char msg_len[HDR_MSGSIZE_LEN+1];
    memset(msg_len, 0, HDR_MSGSIZE_LEN+1);
    strncpy(msg_len, buffer+1, HDR_MSGSIZE_LEN);

    return atoi(msg_len);
}

void read_msg(int fd, int bufsize, char *body, int *body_len, int *opcode) {
    char buffer[bufsize];
    memset(buffer, 0, bufsize);
}