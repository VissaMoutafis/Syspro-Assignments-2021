#include "Networking.h"

int main(int argc, char **argv) {
    struct hostent *mypc = get_ip(argv[1]);
    struct in_addr **ips = (struct in_addr **)mypc->h_addr_list;
    for (int i = 0; ips[i]; i++)
        printf("'%s' -> %s\n", mypc->h_name, inet_ntoa(*ips[i]));

    int sock = -1;
    in_addr_t ipaddr = ntohl(ips[0]->s_addr);
    int port = atoi(argv[2]);
    char *ip = inet_ntoa(*ips[0]);

    // parent (server)
    sock = create_socket();
    bind_socket_to(sock, ipaddr, port);
    listener_set_up(sock, 2);
    while (1) {
        // wait to accept a connection 
        int sockfd = wait_connection(sock, NULL, NULL);
        printf("New fd: %d\n", sockfd);
        // read the request and process it (just print it) 
        char *request = NULL;
        int req_len = 0;
        int opcode = -1;
        net_read_msg(sockfd, 1, &request, &req_len, &opcode);   
        char buf[1000];
        memset(buf, 0, 1000);
        memcpy(buf, request, req_len);
        printf("Server: Received packet<%d|%0*d|%s>\n", opcode, 10, req_len, buf);
        
        // reply to client
        char reply[8];
        sprintf(reply, "MSG-OK");
        net_send_msg(sockfd, 100, reply, 7, opcode);

        // close connection
        if (close(sockfd) < 0)
            puts("Failed to shutdown communication sock");
        if (!strcmp(buf, "END\n")) break;
    }
    if (shutdown(sock, SHUT_RDWR) < 0) puts("Failed to shutdown listener sock");

}