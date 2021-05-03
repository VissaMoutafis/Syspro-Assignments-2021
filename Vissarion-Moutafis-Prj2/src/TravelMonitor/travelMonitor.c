#include "TravelMonitor.h"
#include "StructManipulation.h"

// Basic Utilities




// Travel Monitor Routines

TravelMonitor travel_monitor_create(char *input_dir, size_t bloom_size, int num_monitors, u_int32_t buffer_size) {
    TravelMonitor monitor = calloc(1, sizeof(*monitor));

    monitor->buffer_size = buffer_size;
    monitor->num_monitors = num_monitors;
    monitor->bloom_size = bloom_size;
    monitor->accepted = 0;
    monitor->rejected = 0;
    monitor->manager = monitor_manager_create(num_monitors);
    monitor->virus_stats = ht_create(virus_stats_compare, virus_stats_hash, virus_stats_destroy);

    bool initialization_success = initialization(monitor, input_dir);

    if (!initialization_success) {
        fprintf(stderr, "Travel Monitor intialization failed. Exiting...\n");
        exit(1);
    }

    return monitor;
}


void travel_monitor_destroy(TravelMonitor m) {
    monitor_manager_destroy(m->manager);
    ht_destroy(m->virus_stats);
    free(m);
}

int main(void) {
    TravelMonitor m = travel_monitor_create("testdir", 40, 1, 7);
    travel_monitor_destroy(m);
}