/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/

#include "Networking.h"

static char error_str[200];

struct hostent *get_ip(void) {
    char hostname[BUFSIZ];
    memset(hostname, 0, BUFSIZ);
    gethostname(hostname, BUFSIZ);
    struct hostent *machine = NULL;
    if ((machine = gethostbyname(hostname)) == NULL)
        fprintf(stderr, "Could not resolve hostname '%s'\n", hostname);

    #ifdef DEBUG
    struct in_addr **ips = (struct in_addr **)machine->h_addr_list;
    printf("%s -> %s\n", hostname, inet_ntoa(*ips[0]));
    #endif
    return machine;
}

// create a socket and return <socket fd> in success and -1 otherwise
int create_socket(void) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) // check if we failed
        perror("create_socket");
    // return the socket fd
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR)");
    return sock_fd;
}

// bind the socket to a local address:port
void bind_socket_to(int sock, in_addr_t ip_addr, int port) {
    // make the socket reusable
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) <0)
        perror("setsockopt(SO_REUSEADDR)");
    // init binder struct
    struct sockaddr_in binder;
    memset(&binder, 0, sizeof(struct sockaddr_in));
    binder.sin_family = AF_INET;                // ipv4
    binder.sin_addr.s_addr = htonl(ip_addr);    // ipaddr into network encoding
    binder.sin_port = htons(port);              // port num into network encoding
    if (bind(sock, (struct sockaddr *)&binder, sizeof(binder)) < 0) {
        sprintf(error_str, "bind failed for socket: %d on %s:%d", 
                    sock, inet_ntoa(binder.sin_addr), port);
        perror(error_str);
        exit(1);
    }
}

// connect to ip:port with <sock>
int connect_to(int sock, in_addr_t ip_addr, int port) {
    // set the sockaddr struct of the machine we wanna connect to 
    struct sockaddr_in machine;
    memset(&machine, 0, sizeof(struct sockaddr_in));
    machine.sin_family = AF_INET;               // ipv4
    machine.sin_addr.s_addr = htonl(ip_addr);   // ipaddr into network encoding
    machine.sin_port = htons(port);             // port num into network encoding

    // perform a connection and return the connect's syscall return code
    
    int ret = -1;
    int tries = MAX_CONNECT_TRIES;
    do {
        #ifdef DEBUG
        printf("(%d)Trying to connect %s:%d\n", MAX_CONNECT_TRIES-tries, inet_ntoa(machine.sin_addr), port);
        #endif

        ret = connect(sock, (struct sockaddr *)&machine, sizeof(machine));   
        tries --;
        if (tries % 10 == 0) sleep(1); // every 10 tries sleep for a sec
    } while (ret != 0 && tries);

    #ifdef DEBUG
    printf("%s to connect to %s:%d\n", ret < 0 ? "FAILED" : "SUCCEED",
           inet_ntoa(machine.sin_addr), port);
    #endif
    if (ret == -1) {
        fprintf(stderr, "Failed to connect to  %s:%d (TIMEOUT OCCURED)\n",inet_ntoa(machine.sin_addr), port);
    }
    return ret;
}

// wrapper for listen syscall
void listener_set_up(int sockfd, int backlog) {
    if (listen(sockfd, backlog) < 0) {
        char error_str[100];
        sprintf(error_str, "Failed to set up listener (%d con's) for socket (%d)", backlog, sockfd);
        perror(error_str);
    }
}

// wait for a connection in <ip_addr>:<port>
// return a new sockfd on success and -1 on error
int wait_connection(int sockfd, struct sockaddr_in *_machine, socklen_t *_machine_size) {
    int newfd = -1;
    struct sockaddr_in machine;
    memset(&machine, 0, sizeof(machine));
    socklen_t machine_size = sizeof(machine);
    if ((newfd = accept(sockfd, (struct sockaddr *)&machine, &machine_size)) < 0) {
        perror("accept @ wait_connection");
    }

    // check if the user actually wants to get the machine
    if (_machine) {
        memcpy(_machine, &machine, machine_size);
        *_machine_size = machine_size;
    }

    #ifdef DEBUG
    if (newfd)
        printf("Accepted connection with new-socket %d from %s\n", newfd, 
            inet_ntoa(machine.sin_addr));
    #endif

    return newfd; // return either the new fd (on success) or -1 (on error)
}

int net_read_msg(int sockfd, u_int32_t bufsize, char **msg, int *msg_len, int *opcode) {
    // generic wrapper of read_msg
    
    return read_msg(sockfd, bufsize, msg, msg_len, opcode);
}

void net_send_msg(int sockfd, uint32_t bufsize, char *msg, int msg_len, int opcode) {
    // generic wrapper of send_msg
    send_msg(sockfd, bufsize, msg, msg_len, opcode);
}

int get_unique_port(void) {
    static int port = NET_LOWEST_PORT;

    // if we reach the max system ports just return the same port from now on
    if (port > NET_HIGHEST_PORT){
        fprintf(stderr, "No more ports available. Returning the same port from now on.\n");
        return port;
    }

    // increase port id.
    port ++;
    return port;
}
