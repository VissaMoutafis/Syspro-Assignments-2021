/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#include "Setup.h"
#include <math.h>

bool assign_dirs(TravelMonitor monitor, char *input_dir) {
    DIR *dirp = opendir(input_dir);
    if (dirp == NULL) {
        char error_buf[BUFSIZ];
        sprintf(error_buf, "Problem opening directory %s", input_dir);
        perror(error_buf);
        return false;
    } else {
        struct dirent **dir_array = NULL;
        int num_elems = 0;
        num_elems = scandir(input_dir, &dir_array, NULL, alphasort);
        int monitor_id = 0;
        int num_monitors = monitor->num_monitors; 
        for (int i = 0; i < num_elems; i++) {
            if (!strcmp(dir_array[i]->d_name, ".") || !strcmp(dir_array[i]->d_name, "..")) {free(dir_array[i]);continue;}
            // get the dir path
            char country_path[BUFSIZ];
            memset(country_path, 0, BUFSIZ);
            sprintf(country_path, "%s/%s", input_dir, dir_array[i]->d_name);

            // assign it to the respective monitor (round robin wise)
            monitor_manager_add_country(monitor->manager, monitor_id, country_path);
            // get the next monitor
            monitor_id = (monitor_id + 1) % num_monitors;

            //we no longer need the dir[i]
            free(dir_array[i]);
        }
        free(dir_array);
    }
    // close the dir
    closedir(dirp);

    return true;
}

void call_server(TravelMonitor monitor, int i, int port) {
    // first we have to set the standard argv
    char bufsize[20]; sprintf(bufsize, "%u", monitor->buffer_size);
    char circularbuffersize[20]; sprintf(circularbuffersize, "%u", monitor->circular_buffer_size);
    char bloomsize[20]; sprintf(bloomsize, "%lu", monitor->bloom_size);
    char portnum[20]; sprintf(portnum, "%d", port);
    char numthreads[20]; sprintf(numthreads, "%d", monitor->num_threads);

    // Now we wait for the assigned dirs from the parent process
    int len = monitor->manager->monitors[i].num_countries;
    char **dirs = monitor->manager->monitors[i].countries_paths;

    // Now we have to set up an argument vector for exec
    char *argv[1+10+len+1];
    argv[0] = "./monitorServer";
    argv[1] = "-p"; argv[2] = portnum;
    argv[3] = "-t"; argv[4] = numthreads;
    argv[5] = "-b"; argv[6] = bufsize;
    argv[7] = "-c"; argv[8] = circularbuffersize;
    argv[9] = "-s"; argv[10] = bloomsize;
    // copy countries
    memcpy(argv+11, dirs, len*sizeof(char *));
    argv[len+11] = NULL;

    #ifdef DEBUG
    printf("%s\nready to call:\n ", bloomsize);
    for (int i = 0; i < 1+10+len; i++) printf("%s ", argv[i]);
    puts("");
    #endif

    // execute the call
    if (execv("./monitorServer", argv) < 0) {perror("Failed exec'ing child"); exit(1);}
}


// routine to fork a monitor process
bool create_monitor(TravelMonitor monitor, bool update, int i) {
    // get a unique port
    int port = get_unique_port();
    // fork the child
    pid_t pid = fork();
    
    switch (pid) {
        case -1:  // error behaviour
            fprintf(stderr, "Cannot fork child\n");
            return false;
        break;

        case 0:  // child behaviour
            call_server(monitor, i, port); // server set up and actual exec-call
        break;

        default:  // parent behaviour
            // add the monitor into the manager 
            // and add a specific unique port that you know it is listening
            monitor_manager_add(monitor->manager, pid, port);

            #ifdef DEBUG
            printf("Added server at port (%d), PID: %d \n", port, pid);
            #endif
        break;
    }

    return true;
}

bool create_n_monitors(TravelMonitor monitor) {
    bool all_ok = true;
    for (int i = 0; i < monitor->num_monitors; i++) {
        if (!create_monitor(monitor, false, i)) 
            all_ok = false;
    }
    return all_ok;
}

bool create_logs(void) {
    // we have to check if the logs directories are created. if they are we delete them and then create empty ones
    if (access(ROOT_LOG_PATH, F_OK) == 0) delete_element(ROOT_LOG_PATH);

    bool success = mkdir(ROOT_LOG_PATH, 0777) != -1
                && mkdir(TRAVEL_MONITOR_LOG_PATH, 0777) != -1
                && mkdir(MONITOR_LOG_PATH, 0777) != -1;
    return success;
}

bool initialization(TravelMonitor monitor, char *input_dir) {
    // create the log files
    // assign the dirs
    // for the monitors 
    // pass them args and execute the monitorServer binary
    // wait for the response to initialize the travel monitor client (blooms)
    bool all_ok = true;
    all_ok = all_ok && create_logs();
    all_ok = all_ok && assign_dirs(monitor, input_dir);
    all_ok = all_ok && create_n_monitors(monitor);
    // Now we have to get all the bloom filters.
    int sockfds[monitor->num_monitors]; // the array of socket file descriptors
    
    // we will create a connection with all of the monitor servers and save the socket fds in the <sockfds>
    for (int i = 0; i < monitor->num_monitors; i++) {
        // create a socket and get the relative variables for the connection
        int sockfd = create_socket();
        sockfds[i] = sockfd;
        int port = monitor->manager->monitors[i].port;

        // try to connect to a ipaddr:port 
        if (connect_to(sockfd, _ip_addr_, port) < 0) {
            fprintf(stderr, "Could not connect to server-%d (%s, %d)\n", 
                i,
                inet_ntoa((struct in_addr){.s_addr=htonl(_ip_addr_)}), 
                port);
            exit(1);
        }
    }
    // wait for the bloom filters
    all_ok = !travel_monitor_get_response(monitor->buffer_size, monitor, get_bf_from_child, -1, sockfds, NULL); 

    // close all of the connections
    for (int i = 0; i < monitor->num_monitors; i++) shutdown(sockfds[i], SHUT_RDWR);

    return all_ok;
}