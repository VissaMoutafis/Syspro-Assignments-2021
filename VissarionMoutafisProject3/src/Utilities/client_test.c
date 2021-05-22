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
    int i = 0;
    while (1) {
        // get message
        char msg[100];
        memset(msg, 0, 100);
        fgets(msg, 100, stdin);
    
        // create socket
        sock = create_socket();
        printf("New fd: %d\n", sock);
        // connect to server
        int ret;
        do {
            ret = connect_to(sock, ipaddr, port);
        } while (ret != 0);
        printf("Client: Connected to %s:%d\n", ip, port);

        // send the request
        net_send_msg(sock, 3, msg, strlen(msg), i);

        // wait to receive the request
        char *response = NULL;
        int len = -1;
        int opcode = -1;
        net_read_msg(sock, 17, &response, &len, &opcode);
        char b[100];
        memset(b, 0, 100);
        memcpy(b, response, len);
        printf("Client: Server responsed <%d|%0*d|%s>\n", opcode, 10, len, b);

        // close socket
        if (shutdown(sock, SHUT_RDWR) < 0) puts("Failed to shutdown");
        if (close(sock) < 0) puts("Too many closes");
        i = (i+1)%sizeof(char);
        if (!strcmp(msg, "END\n")) break;
    }
}
