#include "TravelMonitor.h"
#include "IPC.h"

// Basic Utilities
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
            if (!strcmp(dir_array[i]->d_name, ".") || !strcmp(dir_array[i]->d_name, ".."))continue;
            // get the dir path
            char country_path[BUFSIZ];
            memset(country_path, 0, BUFSIZ);
            sprintf(country_path, "%s/%s", input_dir, dir_array[i]->d_name);

            // assign it to the respective monitor (round robin wise)
            monitor_manager_add_country(monitor->manager, monitor_id, country_path);
            // get the next monitor
            monitor_id = (monitor_id + 1) % num_monitors;
        }
    }

    // close the dir
    closedir(dirp);

    // sent the assigned countries paths to monitors
    for (int i = 0; i < monitor->num_monitors; i++) {
        MonitorTrace t;
        memset(&t, 0, sizeof(MonitorTrace));
        // get the trace at i-th position
        if (!monitor_manager_get_at(monitor->manager, i, &t)) {
            fprintf(stderr, "Error in getting the %d-th monitor, at dir assignment.\n", i);
            exit(1);
        }
        char **countries = t.countries_paths;
        int num_countries = t.num_countries;
        printf("Statring to send countries to monitor %d\n", i);

        for (int j = 0; j < num_countries; j++) {
            // send the country to the child
            printf("sending %s, via %d\n", countries[j], t.out_fifo);
            send_msg(t.out_fifo, countries[j], strlen(countries[j]), INIT_CHLD);
        }
    } 
    return true;
}



// routine to fork a monitor process
bool create_monitor(TravelMonitor monitor, int i) {
    char to_fifo_path[BUFSIZ], from_fifo_path[BUFSIZ];
    memset(to_fifo_path, 0, BUFSIZ);
    memset(from_fifo_path, 0, BUFSIZ);
    // create the communication fifos
    create_unique_fifo_pair(false, i, from_fifo_path, to_fifo_path);
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
            execl("./monitor", "./monitor", "-i", to_fifo_path, "-o", from_fifo_path, NULL);

            break;

        default:  // parent behaviour
            // add the monitor into the manager
            in_fifo = open(from_fifo_path, O_RDONLY | O_NONBLOCK);
            out_fifo = open(to_fifo_path, O_WRONLY);
            printf("Parent reading from %s (%d) and writing to %s (%d)\n", from_fifo_path, in_fifo, to_fifo_path, out_fifo);
            monitor_manager_add(monitor->manager, pid, in_fifo, out_fifo);

            #ifdef DEBUG
            MonitorTrace t;
            assert(monitor_manager_search_pid(monitor->manager, pid, &t) >= 0);
            #endif
            break;
    }

    return true;
}

bool create_n_monitors(TravelMonitor monitor) {
    puts("create");
    bool all_ok = true;
    // create the fifo dir
    create_unique_fifo_pair(true, -1, NULL, NULL);
    for (int i = 0; i < monitor->num_monitors; i++) {
        if (!create_monitor(monitor, i)) 
            all_ok = false;
    }
    return all_ok;
}

static bool get_children_response(TravelMonitor monitor) {
    puts("response");
    // we will use poll to monitor the fifos
    // first set up the fds
    int nfd = monitor->num_monitors;
    struct pollfd fds[nfd];
    for (int i = 0; i < nfd; i++) {
        MonitorTrace t;
        memset(&t, 0, sizeof(Trace));
        if (!monitor_manager_get_at(monitor->manager, i, &t)){
            fprintf(stderr, "monitor_manager_get_at: Cannot get child at %d\n", i);
            exit(1);
        }
        fds[i].fd = t.in_fifo;
        fds[i].events = POLL_IN;
    }
    int active = nfd;
    while (active) {
        int ret = poll(fds, nfd, 1);
        if (ret) {
            for (int i = 0; i < nfd; i++) {
                // we can read
                if ((fds[i].revents & POLL_IN) == POLL_IN) {
                    // there's something to read
                    char *msg = NULL;
                    int len = 0;
                    int opcode = -1;
                    read_msg(fds[0].fd, 1000, &msg, &len, &opcode);
                    if (len == 0) {
                        // process do not write anymore so just leave
                        active --;
                        continue;
                    }
                    // some handling
                    char buf[BUFSIZ];
                    memset(buf, 0, BUFSIZ);
                    sprintf(buf, "opcode: %d, msg-len: %d, msg: ", opcode, len);
                    memcpy(buf+31, msg, len);
                    puts(buf);
                    // end of handling

                    free(msg);
                }
            }
        }
        if (ret < 0){perror("poll"); exit(1);}
    }
    return true;
}

bool initialization(TravelMonitor monitor, char *input_dir) {
    // initialization components
    // 2. Create fifos and fork the monitor processes
    // 3. Assign them the directories
    return create_n_monitors(monitor) 
        && assign_dirs(monitor, input_dir)
        && get_children_response(monitor); 
}

// Travel Monitor Routines

TravelMonitor travel_monitor_create(char *input_dir, size_t bloom_size, int num_monitors, u_int32_t buffer_size) {
    TravelMonitor monitor = calloc(1, sizeof(*monitor));

    monitor->buffer_size = buffer_size;
    monitor->num_monitors = num_monitors;
    monitor->accepted = 0;
    monitor->rejected = 0;
    monitor->manager = monitor_manager_create(num_monitors);
    // monitor->virus_stats = ht_create();

    bool initialization_success = initialization(monitor, input_dir);

    if (!initialization_success) {
        fprintf(stderr, "Travel Monitor intialization failed. Exiting...\n");
        exit(1);
    }

    return monitor;
}


// test for I/O functions
int main(void) {
    TravelMonitor m = travel_monitor_create("testdir", 1000, 1, 7);
}