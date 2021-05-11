#include "TravelMonitor.h"
#include "Setup.h"
#include "TTY.h"

static char *usage = "Usage: \n    ~$ ./travelMonitor –m numMonitors -b bufferSize -s sizeOfBloom -i input_dir";

int main(int argc, char ** argv) {
    printf("parent %d\n", getpid());
    char *values[4] = {NULL, NULL, NULL, NULL};
    char *allowed_args[] = {"-m", "-b", "-s", "-i"};
    if (!parse_args(argc, argv, values, allowed_args, 4)
        || !is_numeric(values[0])                            
        || !is_numeric(values[1]) 
        || !is_numeric(values[2])) 
    {
        print_arg_error(usage);
        exit(1);
    }
    
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

    size_t sizeOfBloom = DEF_BLOOM_SIZE;
    if (atoi(values[2]) > 0)
        sizeOfBloom = atoi(values[2]);
    else
        fprintf(stderr, "The sizeOfBloom arg is non-positive. Falling back to default: %u", DEF_BLOOM_SIZE);

    char *input_dir = values[3];
    if (access(input_dir, F_OK) != 0 || !is_dir(input_dir)) {
        fprintf(stderr, "'%s' cannot be opened as a dir\n", input_dir);
        exit(1);
    }

    // initialize signal handlers
    travel_monitor_signal_handlers();
    // intialize monitor structs
    travel_monitor_initialize();
    // Create a monitor
    TravelMonitor monitor = travel_monitor_create(input_dir, sizeOfBloom, numMonitors, bufferSize);

    // send a syn packet to all monitors and after that wait for a ack
    for (int i = 0; i < monitor->manager->num_monitors; i++) {
        bool ack_received = false;
        void *ret_args[] = {&ack_received};
        MonitorTrace *m_trace = &(monitor->manager->monitors[i]);
        send_msg(m_trace->out_fifo, NULL, 0, SYN_OP);
        
        travel_monitor_get_response(bufferSize, monitor, accept_ack, m_trace->in_fifo, ret_args);
        if (!ack_received) {
            printf("process %d is not ready.\n", m_trace->pid);
        }
    }

    while (!is_end && !sigint_set && !sigquit_set) {
        // first get the input expression from the tty
        char *expr = NULL;
        if (sigchld_set) {
            // SIGCHLD handler
            travel_monitor_restore_children(monitor);
            sigchld_set = false;
        } else {
            expr = get_input();
        }

        if (errno == EINTR) {
            // clean errno
            errno = 0;
            // clean the buffer
            if (expr) free(expr);
            // clean stdin
            clean_stream(&stdin);
            // check the signals again
            continue;
        }

        // now parse it to the expression part and the value part
        char **parsed_expr = expr ?
            parse_expression(expr)
            : NULL;  // format: /command value(s)

        // clarify the input with proper assignments
        char *command = parsed_expr ? parsed_expr[0] : NULL;
        char *value = parsed_expr ? parsed_expr[1] : NULL;
        int expr_index;

        // CREATE THE VACCINE MONITOR

        // now we have to check if the expression was ok based on the array of
        // allowed formats
        
        if (command) {    
            if (check_format(command, &expr_index) &&
                check_value_list(value, expr_index)) {
                // we will try to execute the command. If vaccine monitor fails then
                // we will print the error message to stderr
                if (!travel_monitor_act(monitor, expr_index, value)) {
                    fprintf(stderr, "%s\n", error_msg);
                }
            } else
                help();
            if (expr) free(expr);
            if (parsed_expr[0]) free(parsed_expr[0]);
            if (parsed_expr[1]) free(parsed_expr[1]);
            if (parsed_expr) free(parsed_expr);
        }
    }

    if (!is_end)
        travel_monitor_finalize(monitor);
    
    travel_monitor_destroy(monitor);
}