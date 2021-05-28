/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/

#pragma once

// my headers
#include "Types.h"
#include "IPC.h"

// system headers for socket communication
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// Some utility wrappers for networking routines 

// Get the hostent pointer from your host.
// Return the staticaly allocated pointer in success, else NULL.
struct hostent *get_ip(void);

// Create a TCP socket of AF_INET domain.
// Returns the descriptor in success, else returns -1.
int create_socket(void);

// Bind <sock> to local address <ip_addr> on <port>.
// <ip_addr> must be in host format (not network)
void bind_socket_to(int sock, in_addr_t ip_addr, int port);

// Connect to ip:port via <sock>'s interface
// Return 0 on success and -1 on error
// <ip_addr> must be in host format (not network)
int connect_to(int sock, in_addr_t ip_addr, int port);

// wrapper for listen syscall
void listener_set_up(int sockfd, int backlog);

// Wait for a connection. (accept wrapper)
// Returns the <newfd> on success, otherwise returns -1
// <machine> and <machine_size> are staticaly allocated passed by address
// If you want to ignore them then just pass machine=NULL, machine_size=NULL.
int wait_connection(int sockfd, struct sockaddr_in *machine, socklen_t *machine_size);
// send a <msg> of <msg_len> and <opcode> into <sockfd> socket by <bufsize> bytes per I/O

// Read a <msg> of <msg_len> and <opcode> from <sockfd> socket by <bufsize>
// bytes per I/O
int net_read_msg(int sockfd, u_int32_t bufsize, char **msg, int *msg_len, int *opcode);

// Send a <msg> of <msg_len> and <opcode> into <sockfd> socket by <bufsize> bytes per I/O
void net_send_msg(int sockfd, uint32_t bufsize, char *msg, int msg_len, int opcode);

// Get a unique port number of all available ports defined in this header
int get_unique_port(void);

// DEFINE CONFIGURATION CONSTANTS //

// these define the port range that the user chooses 
// in this range the ports of the monitorServers will be chosen
#define NET_LOWEST_PORT 50500
#define NET_HIGHEST_PORT 65535

// max tries to connect to a device
#define MAX_CONNECT_TRIES 100