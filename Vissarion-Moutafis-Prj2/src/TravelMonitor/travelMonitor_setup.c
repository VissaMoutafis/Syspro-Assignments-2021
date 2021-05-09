#include "Setup.h"

//routine to create fifos
void create_unique_fifo_pair(bool init, char *from, char *to) {
    if (init) {
        // check if the fifo dir exists. create it. return;
        if (access(FIFO_DIR, F_OK) == 0) delete_dir(FIFO_DIR);

        if (mkdir(FIFO_DIR, 0777) == -1) {
            perror("mkdir at fifo dir creation");
            exit(1);
        }
        return;
    }
    static int unique_id = 0;

    sprintf(to, "%s/to-monitor-%d.fifo", FIFO_DIR, unique_id);
    if (mkfifo(to, 0777) == -1) {perror("mkfifo"); exit(1);}
    sprintf(from, "%s/from-monitor-%d.fifo", FIFO_DIR, unique_id);
    if (mkfifo(from, 0777) == -1) {perror("mkfifo"); exit(1);}

    unique_id++;
}

void clean_fifos(void) {
    // check if the fifo dir exists. create it. return;
    if (access(FIFO_DIR, F_OK) == 0) delete_dir(FIFO_DIR);
    else {perror("clean fifos"); exit(1);}
}

void send_dirs_to_monitor(MonitorTrace *t) {
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
    send_msg(t->out_fifo, buffer, bufsiz, INIT_CHLD);
    // send end message code (<msg> set to NULL)
    send_msg(t->out_fifo, NULL, 0, MSGEND_OP);
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
        send_dirs_to_monitor(&t);
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

// routine to fork a monitor process
bool create_monitor(TravelMonitor monitor, bool update, int i) {
    char to_fifo_path[BUFSIZ], from_fifo_path[BUFSIZ];
    memset(to_fifo_path, 0, BUFSIZ);
    memset(from_fifo_path, 0, BUFSIZ);
    // create the communication fifos
    create_unique_fifo_pair(false, from_fifo_path, to_fifo_path);
    int in_fifo, out_fifo;

    // fork the child
    pid_t pid = fork();

    // insert it in the monitor manager and assign it fifos
    switch (pid) {
        case -1:  // error behaviour
            fprintf(stderr, "Cannot fork child\n");
            return false;
        break;

        case 0:  // child behaviour
            // set the args for current child-process and call exec
            if (execl("./monitor", "./monitor", "-i", to_fifo_path, "-o", from_fifo_path, NULL) == -1) {
                perror("execl"); exit(1);
            }
        break;

        default:  // parent behaviour
            // add the monitor into the manager
            in_fifo = open(from_fifo_path, O_RDONLY | O_NONBLOCK);
            out_fifo = open(to_fifo_path, O_WRONLY);
            if (in_fifo < 0 || out_fifo < 0) {
                perror("open"); exit(1);
            }
            if (!update) 
                monitor_manager_add(monitor->manager, pid, in_fifo, out_fifo);
            else {
                monitor->manager->monitors[i].pid = pid;
                monitor->manager->monitors[i].in_fifo = in_fifo;
                monitor->manager->monitors[i].out_fifo = out_fifo;
            }

            #ifdef DEBUG
            printf("infifo: %s - %d\noutfifo: %s - %d\n", from_fifo_path, in_fifo, to_fifo_path, out_fifo);
            #endif
        break;
    }

    return true;
}

bool create_n_monitors(TravelMonitor monitor) {
    bool all_ok = true;
    // create the fifo dir
    create_unique_fifo_pair(true, NULL, NULL);
    for (int i = 0; i < monitor->num_monitors; i++) {
        if (!create_monitor(monitor, false, -1)) 
            all_ok = false;
    }
    return all_ok;
}

void send_init_stats_to_monitor(TravelMonitor monitor, MonitorTrace *t) {
    char buf[10 + 1 + 10];
    memset(buf, 0, 21);
    snprintf(buf, 21, "%0*u%0*lu", 10, monitor->buffer_size, 10,
             monitor->bloom_size);
    printf("sending stuff to %d\n", t->out_fifo);
    // send the init elements
    send_msg(t->out_fifo, buf, 20, INIT_CHLD);
    // communicate transmision termination
    send_msg(t->out_fifo, NULL, 0, MSGEND_OP);
}

bool send_init_stats(TravelMonitor monitor) {
    // we will pass the msg:
    // <buffer size>$<bloom size> : max length = 10 + 10 (INT MAX length is 10)
    

    for (int i = 0; i < monitor->num_monitors; i++) {
        MonitorTrace t;
        if (!monitor_manager_get_at(monitor->manager, i, &t)) {
            fprintf(stderr, "send_init_stats: Cannot get monitor stats of %d-th monitor\n", i);
            exit(1);
        }
        send_init_stats_to_monitor(monitor, &t);
    }
    return true;
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
    // fork monitors
    // send them init stats
    // assign them directories
    // send them the assigned dirs
    // wait for the response to initialize the travel monitor
    return create_logs()
        && create_n_monitors(monitor) 
        && send_init_stats(monitor)
        && assign_dirs(monitor, input_dir)
        && send_dirs(monitor)
        && travel_monitor_get_response(monitor->buffer_size, monitor, get_bf_from_child, -1, NULL); 
}