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

// Get the hostent pointer from a host that resolves it to an IPv4 address.
// Return the staticaly allocated pointer in success, else NULL.
struct hostent *get_ip(char *hostname);

// Create a TCP socket of AF_INET domain.
// Returns the descriptor in success, else returns -1.
int create_socket(void);

// Bind <sock> to local address <ip_addr> on <port>.
void bind_socket_to(int sock, in_addr_t ip_addr, int port);

// Connect to ip:port via <sock>'s interface
// Return 0 on success and -1 on error
int connect_to(int sock, in_addr_t ip_addr, int port);

// wrapper for listen syscall
void listener_set_up(int sockfd, int backlog);

// DEFINE CONFIGURATION CONSTANTS //

// these define the port range that the user chooses 
// in this range the ports of the monitorServers will be chosen
#define NET_LOWEST_PORT 50000
#define NET_HIGHEST_PORT 65535

