/**
 *	Syspro Project 3
 *	 Written By Vissarion Moutafis sdi1800119
 **/

#include "Setup.h"
#include "TTY.h"
#include "TravelMonitor.h"

static char *usage =
    "Usage: \n    ~$ ./travelMonitorClient -m numMonitors "
    "-b socketBufferSize "
    "-c cyclicBufferSize "
    "-s sizeOfBloom "
    "-i input_dir "
    "-t numThreads ";

// call as ./travelMonitorClient –m numMonitors -b socketBufferSize -c
// cyclicBufferSize -s
// sizeOfBloom -i input_dir -t numThreads
int main(int argc, char **argv) {
    #ifdef DEBUG
    printf("parent-client %d\n", getpid());
    #endif

    char *values[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
    char *allowed_args[] = {"-m", "-b", "-c", "-s", "-i", "-t"};
    if (!parse_args(argc, argv, values, allowed_args, 6) ||
        !is_numeric(values[0]) || !is_numeric(values[1]) ||
        !is_numeric(values[2]) || !is_numeric(values[3]) ||
        !is_numeric(values[5])) {
        print_arg_error(usage);
        exit(1);
    }

    // some variable checking

    // if the checks are ok then go set the appropriate variables
    int numMonitors = DEF_NUM_MONITORS;
    if (atoi(values[0]) > 0)
        numMonitors = atoi(values[0]);
    else
        fprintf(stderr, "The numMonitors arg is non-positive. Falling back to default: %d", DEF_NUM_MONITORS);

    u_int32_t bufferSize = DEF_BUFFER_SIZE;
    if (atoi(values[1]) > 0)
        bufferSize = atoi(values[1]);
    else
        fprintf(stderr, "The bufferSize arg is non-positive. Falling back to default: %u", DEF_BUFFER_SIZE);

    int cyclicBufferSize = DEF_CBUF_SIZE;
    if (atoi(values[2]) > 0)
        cyclicBufferSize = atoi(values[2]);
    else
        fprintf(stderr, "The cyclicBufferSize arg is non-positive. Falling back to default: %d", DEF_CBUF_SIZE);

    size_t sizeOfBloom = DEF_BLOOM_SIZE;
    if (atoi(values[3]) > 0)
        sizeOfBloom = atoi(values[3]);
    else
        fprintf(stderr, "The sizeOfBloom arg is non-positive. Falling back to default: %u", DEF_BLOOM_SIZE);

    char *input_dir = values[4];
    if (access(input_dir, F_OK) != 0 || !is_dir(input_dir)) {
        fprintf(stderr, "'%s' cannot be opened as a dir\n", input_dir);
        exit(1);
    }

    int numThreads = DEF_NUM_THREADS;
    if (atoi(values[5]) > 0)
        numThreads = atoi(values[5]);
    else
        fprintf(stderr, "The numThreads arg is non-positive. Falling back to default: %d", DEF_NUM_THREADS);

    // intialize monitor structs
    travel_monitor_initialize();
    
    // Create a monitor
    TravelMonitor monitor = travel_monitor_create(input_dir, sizeOfBloom, numMonitors, bufferSize, cyclicBufferSize, numThreads);

    // send a syn packet to all monitors and after that wait for a ack
    for (int i = 0; i < monitor->manager->num_monitors; i++) {
        // create connection to send SYN and receive ACK
        connection_sockfd = create_socket();

        if (connect_to(connection_sockfd, _ip_addr_, monitor->manager->monitors[i].port) < 0) {
            fprintf(stderr, "Failed to connect to server at port %d\n", _port_);
            exit(1);
        }

        bool ack_received = false;
        void *ret_args[] = {&ack_received};
        MonitorTrace *m_trace = &(monitor->manager->monitors[i]);
        send_msg(connection_sockfd, monitor->buffer_size, NULL, 0, SYN_OP);

        travel_monitor_get_response(bufferSize, monitor, accept_ack, connection_sockfd, NULL, ret_args);
        if (!ack_received) {
            printf("process %d is not ready.\n", m_trace->pid);
        }

        if (shutdown(connection_sockfd, SHUT_RDWR) < 0) {perror("PARENT FAILED TO CLOSE SOCK (INIT)."); exit(1);}
    }

    // while (!is_end) {
    //     // first get the input expression from the tty
    //     char *expr = NULL;
    //     expr = get_input();

    //     if (errno == EINTR) {
    //         // clean errno
    //         errno = 0;
    //         // clean the buffer
    //         if (expr) free(expr);
    //         // clean stdin
    //         clean_stream(&stdin);
    //         // check the signals again
    //         continue;
    //     }

    //     // now parse it to the expression part and the value part
    //     char **parsed_expr =
    //         expr ? parse_expression(expr) : NULL;  // format: /command value(s)

    //     // clarify the input with proper assignments
    //     char *command = parsed_expr ? parsed_expr[0] : NULL;
    //     char *value = parsed_expr ? parsed_expr[1] : NULL;
    //     int expr_index;

    //     // CREATE THE VACCINE MONITOR

    //     // now we have to check if the expression was ok based on the array of
    //     // allowed formats

    //     if (command) {
    //         if (check_format(command, &expr_index) &&
    //             check_value_list(value, expr_index)) {
    //             // we will try to execute the command. If vaccine monitor fails
    //             // then we will print the error message to stderr
    //             if (!travel_monitor_act(monitor, expr_index, value)) {
    //                 fprintf(stderr, "%s\n", error_msg);
    //             }
    //         } else
    //             help();
    //         if (expr) free(expr);
    //         if (parsed_expr[0]) free(parsed_expr[0]);
    //         if (parsed_expr[1]) free(parsed_expr[1]);
    //         if (parsed_expr) free(parsed_expr);
    //     }
    // }

    travel_monitor_finalize(monitor);

    travel_monitor_destroy(monitor);

    exit(0);
}