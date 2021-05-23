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

    printf("%s\nready to call:\n ", bloomsize);
    for (int i = 0; i < 1+10+len; i++) printf("%s ", argv[i]);
    puts("");

    exit(0);

    // execute the call
    execv("./monitorServer", argv);
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
    // fork monitors and:
    //      send them init stats
    //      assign them directories
    //      send them the assigned dirs
    // wait for the response to initialize the travel monitor
    return create_logs()                
        && assign_dirs(monitor, input_dir)
        && create_n_monitors(monitor) 
        && !travel_monitor_get_response(monitor->buffer_size, monitor, get_bf_from_child, -1, NULL); 
}