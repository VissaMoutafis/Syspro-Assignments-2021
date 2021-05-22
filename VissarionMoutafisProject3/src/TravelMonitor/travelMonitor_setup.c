/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#include "Setup.h"

void send_dirs_to_monitor(TravelMonitor monitor, MonitorTrace *t) {
    char **countries = t->countries_paths;
    int num_countries = t->num_countries;
    char *buffer = NULL;  // the buffer we will send to child
    int bufsiz = 0;       // the buffer size
    for (int j = 0; j < num_countries; j++) {
        // reconstruct the buffer

        // get the country length
        int country_len = strlen(countries[j]);

        // set-up new buffer (prev_size + country_len + length for
        // separator)
        char *new_buf = calloc(bufsiz + country_len + 1, sizeof(char));
        if (bufsiz) {
            memcpy(new_buf, buffer, bufsiz);
            free(buffer);
        }
        memcpy(new_buf + bufsiz, countries[j], country_len);
        memcpy(new_buf + bufsiz + country_len, SEP, 1);
        // re-set the original <buffer> and <bufsiz>
        buffer = new_buf;
        bufsiz += country_len + 1;
    }
    // send the country to the child
    send_msg(t->out_fifo, monitor->buffer_size, buffer, bufsiz, INIT_CHLD);
    // send end message code (<msg> set to NULL)
    send_msg(t->out_fifo, monitor->buffer_size, NULL, 0, MSGEND_OP);
    // delete the monitor
    free(buffer);
}

bool send_dirs(TravelMonitor monitor) {
    // sent the assigned countries paths to monitors
    for (int i = 0; i < monitor->num_monitors; i++) {
        MonitorTrace t;
        memset(&t, 0, sizeof(MonitorTrace));
        // get the trace at i-th position
        if (!monitor_manager_get_at(monitor->manager, i, &t)) {
            fprintf(stderr,
                    "Error in getting the %d-th monitor, at dir assignment.\n",
                    i);
            exit(1);
        }
        send_dirs_to_monitor(monitor, &t);
    }
    return true;
}

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

void call_server(TravelMonitor monitor, int i) {
    // first we have to assign dirs so that we know what dirs this process will get
    char **dirs = get_dirs();
    int bufsize = ;
    int circlebuffersize = ;
    int bloomsize = ;
    execl("./monitorServer", "./monitorServer" bla bla ...., NULL);
}


// routine to fork a monitor process
bool create_monitor(TravelMonitor monitor, bool update, int i) {
    // fork the child
    pid_t pid = fork();
    switch (pid) {
        case -1:  // error behaviour
            fprintf(stderr, "Cannot fork child\n");
            return false;
        break;

        case 0:  // child behaviour
            call_server(monitor, i); // server set up and actual exec-call
        break;

        default:  // parent behaviour
            // add the monitor into the manager 
            // and add a specific unique port that you know it is listening
            monitor_manager_add(monitor->manager, pid, get_unique_port());

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
        if (!create_monitor(monitor, false, -1)) 
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
        && create_n_monitors(monitor) 
        && !travel_monitor_get_response(monitor->buffer_size, monitor, get_bf_from_child, -1, NULL); 
}