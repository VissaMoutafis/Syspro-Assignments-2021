#include "TravelMonitor.h"

GetResponse get_response = travel_monitor_get_response;

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
        char *buffer = NULL; // the buffer we will send to child
        int bufsiz = 0;      // the buffer size
        for (int j = 0; j < num_countries; j++) {
            //reconstruct the buffer
            
            // get the country length
            int country_len = strlen(countries[j]);

            // set-up new buffer (prev_size + country_len + length for separator)
            char *new_buf = calloc(bufsiz+country_len+1, sizeof(char));
            if (bufsiz) {
                memcpy(new_buf, buffer, bufsiz);
                free(buffer);
            }
            memcpy(new_buf+bufsiz, countries[j], country_len);
            memcpy(new_buf+bufsiz+country_len, SEP, 1);
            // re-set the original <buffer> and <bufsiz>
            buffer = new_buf;
            bufsiz += country_len+1;
        }
        // send the country to the child
        send_msg(t.out_fifo, buffer, bufsiz, INIT_CHLD);
        // send end message code (<msg> set to NULL)
        send_msg(t.out_fifo, NULL, 0, MSGEND_OP);
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
    bool all_ok = true;
    // create the fifo dir
    create_unique_fifo_pair(true, -1, NULL, NULL);
    for (int i = 0; i < monitor->num_monitors; i++) {
        if (!create_monitor(monitor, i)) 
            all_ok = false;
    }
    return all_ok;
}
bool send_init_stats(TravelMonitor monitor) {
    // we will pass the msg:
    // <buffer size>$<bloom size> : max length = 10 + 10 (INT MAX length is 10)
    char buf[10+1+10];
    memset(buf, 0, 21);
    snprintf(buf, 21, "%0*u%0*lu", 10, monitor->buffer_size, 10, monitor->bloom_size);

    for (int i = 0; i < monitor->num_monitors; i++) {
        MonitorTrace t;
        if (!monitor_manager_get_at(monitor->manager, i, &t)) {
            fprintf(stderr, "send_init_stats: Cannot get monitor stats of %d-th monitor\n", i);
            exit(1);
        }
        // send the init elements
        send_msg(t.out_fifo, buf, 20, INIT_CHLD);
        // communicate transmision termination
        send_msg(t.out_fifo, NULL, 0, MSGEND_OP);
    }
    return true;
}


bool initialization(TravelMonitor monitor, char *input_dir) {
    // initialization components
    // 2. Create fifos and fork the monitor processes
    // 3. Assign them the directories
    return create_n_monitors(monitor) 
        && send_init_stats(monitor)
        && assign_dirs(monitor, input_dir)
        && get_response(monitor->buffer_size, monitor, get_bf_from_child, -1, -1, NULL); 
}

// Travel Monitor Routines

TravelMonitor travel_monitor_create(char *input_dir, size_t bloom_size, int num_monitors, u_int32_t buffer_size) {
    TravelMonitor monitor = calloc(1, sizeof(*monitor));

    monitor->buffer_size = buffer_size;
    monitor->num_monitors = num_monitors;
    monitor->bloom_size = bloom_size;
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


int main(void) {
    TravelMonitor m = travel_monitor_create("testdir", 20, 1, 7);
}