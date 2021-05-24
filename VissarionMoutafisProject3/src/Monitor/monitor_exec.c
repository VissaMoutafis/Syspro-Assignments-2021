/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#include "TTY.h"
#include "Types.h"
#include "Utilities.h"
#include "Monitor.h"
#include "IPC.h"
#include "Setup.h"

GetResponse get_request = monitor_get_response;

int listener_sock=-1;

// call as: ./monitor -p port -t numThreads -b socketBufferSize -c cyclicBufferSize -s sizeOfBloom path1 path2 path3...pathn
int main(int argc, char * argv[]) {
    #ifdef DEBUG
    printf("child-server %d\n", getpid());
    #endif
    
    // set the default values
    char *values[5] = {argv[2], argv[4], argv[6], argv[8], argv[10]};
    // set the directory values
    int dir_num = argc - (1+10);
    char **dirs = dir_num > 0 ? &(argv[11]) : NULL;

    // set up the basic networking vars
    // find ip address of local host since the app is running in the same device
    struct hostent *mypc = get_ip("localhost");
    struct in_addr **ips = (struct in_addr **)mypc->h_addr_list;
    _ip_addr_ = ntohl(ips[0]->s_addr);
    _port_ = atoi(values[0]);
    
    // number of active threads
    int num_threads = atoi(values[1]);

    u_int32_t buffer_size = strtoul(values[2], NULL, 10);
    int cyclic_buffer_size = atoi(values[3]);
    size_t bloom_size = strtoull(values[4], NULL, 10);

    #ifdef DEBUG
    printf("(%d) - buffsiz: %u, bf_size: %lu, cbuffer size: %d\n", getpid(), buffer_size, bloom_size, cyclic_buffer_size);
    #endif
    
    #ifdef DEBUG
    printf("(%d) - countries: %d:\n", getpid(), dir_num);
    for (int i = 0; i < dir_num; i++) puts(dirs[i]);
    #endif

    FM fm = fm_create(dirs, dir_num);

    // initialize the monitor globals
    monitor_initialize();
    // create a monitor 
    Monitor monitor = monitor_create(fm, bloom_size, buffer_size, SL_HEIGHT, SL_FACTOR);

    // create a listener socket and set it up
    listener_sock = create_socket();
    bind_socket_to(listener_sock, _ip_addr_, _port_);
    listener_set_up(listener_sock, 1);
    
    int newsockfd;
    if ((newsockfd = wait_connection(listener_sock, NULL, NULL)) < 0) {fprintf(stderr, "INIT CONNECTION FAILED (BF)\n"); exit(1);}


    // send the bloom filters to the parent process and close the socket
    monitor_send_blooms(monitor, newsockfd); close(newsockfd);

    // Now we have to wait for a syn-packet and then return an ack
    if ((newsockfd = wait_connection(listener_sock, NULL, NULL)) < 0) {
        fprintf(stderr, "INIT CONNECTION FAILED (SYN-ACK)\n");
        exit(1);
    }

    bool is_syn = false;
    void *ret_args[1] = {&is_syn};
    get_request(buffer_size, monitor, accept_syn, newsockfd, NULL, ret_args);
    if (is_syn) {
        send_msg(newsockfd, buffer_size, NULL, 0, ACK_OP);
    } else {
        puts("FAILED");
    }
    // close the socket
    close(newsockfd);

    // HERE WE START THE LOOP FOR THE ACTUAL FUNCTIONALITIES OF THE MONITOR
    // BASIC LOGIC

    // main work-flow    
    while (!is_end) {
        // first create a socket to communicate with the client
        connection_sockfd = create_socket();
        
        // wait for a connection
        if ((connection_sockfd = wait_connection(listener_sock, NULL, NULL)) < 0) {
            fprintf(stderr, "CONNECTION FAILED (QUERY)\nexiting..\n");
            break;
        } // connection done

        char *value = NULL;
        int expr_index = -1;
        void *ret_args2[] = {&expr_index, &value};
        // wait for a request from the parent travel monitor
        int ret = get_request(buffer_size, monitor, get_query, connection_sockfd, NULL, ret_args2);
        
        // process request and answer
        if (ret > 0 && expr_index >= 0) {
            monitor_act(monitor, expr_index, value);
            free(value);
        }

        //close socket 
    }

    // final cleaning function (adjust in the appropriate place of code)
    monitor_destroy(monitor);
    exit(0);
}