#include "TravelMonitor.h"

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
        for (int i = 0; i < num_elems; i++) {
            printf("%s/%s\n", input_dir, dir_array[i]->d_name);
            // HERE CODE TO 
                // get the dir path
                // and assign it to the respective monitor (round robin wise)
                // Finally send the message to the monitor
        }
    }

    // close the dir
    closedir(dirp);
    return true;
}

bool create_fifos(TravelMonitor monitor) {
    // check if the fifo dir exists
    if (access(FIFO_DIR, F_OK) == 0)
        delete_dir(FIFO_DIR);

    if (mkdir(FIFO_DIR, 0777) == -1){perror("mkdir at fifo dir creation");exit(1);}
    exit(0);
    // create monitor->num_monitors * 2 fifos for process communication 
    // and assign them to the monitor manager
    for (int i = 0; i < monitor->num_monitors; i++) {
        char buf[BUFSIZ];
        memset(buf, 0, BUFSIZ);
        sprintf(buf, "%s/to-monitor-%d.fifo", FIFO_DIR, i);
        mkfifo(buf, 0644);
        memset(buf, 0, BUFSIZ);
        sprintf(buf, "%s/from-monitor-%d.fifo", FIFO_DIR, i);
        mkfifo(buf, 0644);
    }
}

bool create_monitor(TravelMonitor monitor, int i) {

}

bool create_n_monitors(TravelMonitor monitor) {
    for (int i = 0; i < monitor->num_monitors; i++) {
        create_monitor(monitor, i);
    }
}

bool initialization(TravelMonitor monitor, char *input_dir) {
    // initialization components
    // 1. Create the fifos
    // 2. Fork the monitor processes and
    // 3. Assign them the directories
    return create_fifos(monitor)
        && create_n_monitors(monitor) 
        && assign_dirs(monitor, input_dir); 
}

// Travel Monitor Routines

TravelMonitor travel_monitor_create(char *input_dir, size_t bloom_size, int num_monitors, u_int32_t buffer_size) {
    TravelMonitor monitor = calloc(1, sizeof(*monitor));

    monitor->buffer_size = buffer_size;
    monitor->num_monitors = num_monitors;
    monitor->accepted = 0;
    monitor->rejected = 0;
    // monitor->manager = monitor_manager_create(num_monitors);
    // monitor->virus_stats = ht_create();

    bool initialization_success = initialization(monitor, input_dir);

    if (!initialization_success) {
        fprintf(stderr, "Travel Monitor intialization failed. Exiting...\n");
        exit(1);
    }
}

int main(void) {
    travel_monitor_create("testdir", 1000, 3, 1000);
}