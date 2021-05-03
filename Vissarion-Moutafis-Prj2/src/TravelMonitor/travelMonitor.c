#include "TravelMonitor.h"
#include "StructManipulation.h"
#include "Setup.h"

bool error_flag = false;
char error_msg[BUFSIZ];
char ans_buffer[BUFSIZ];

static void print_error(bool exit_fail) {
    fprintf(stderr, "%s\n", error_msg);
    error_flag = false;
    memset(error_msg, 0, BUFSIZ);
    if (exit_fail) exit(EXIT_FAILURE);
}

// all the possible commands for the tty API of the app
int pos_cmds_len = 5;

// for format checking
char *allowed_formats[] = {"/travelRequest",
                           "/travelStats",
                           "/addVaccinationRecords",
                           "/searchVaccinationStatus",
                           "exit"};

// for help printing
char *possible_commands[] = {
    "/travelRequest citizenID date countryFrom [countryTo] virusName",
    "/travelStats virusName [date1 date2] [country]",
    "/addVaccinationRecords country",
    "/searchVaccinationStatus citizenID",
    "/exit"};

// for value list argument checking (swallow checks)
int max_values[5] = {5, 4, 1, 1, 0};
int min_values[5] = {4, 1, 1, 1, 0};

void answer(void) {
    puts(ans_buffer);
    memset(ans_buffer, 0, BUFSIZ);
}

// Basic Utilities

// TODO
void travel_request(TravelMonitor monitor, char *value) {
    // value: citizenID date countryFrom [countryTo] virusName

    // if we cannot answer the request here (bf returns true)
    // we delegate to the appropriate process

    if (try_answer_request(monitor, value))
        // the answer is in the <ans_buffer>
        return ;
    else 
        delegate_to_child(monitor, value);
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
    monitor->virus_stats = ht_create(virus_stats_compare, virus_stats_hash, virus_stats_destroy);

    bool initialization_success = initialization(monitor, input_dir);

    if (!initialization_success) {
        fprintf(stderr, "Travel Monitor intialization failed. Exiting...\n");
        exit(1);
    }

    return monitor;
}

bool travel_monitor_act(TravelMonitor monitor, int expr_index, char *value) {
    switch (expr_index) {
        case 0: // comman: /travelRequest, value: citizenID date countryFrom [countryTo] virusName
                travel_request(monitor, value);
                answer();
        break;

        case 1: // command: /travelStats, value: virusName date1 date2 [country]
                travel_stats(monitor, value);
                answer();
        break;

        case 2: // command: /addVaccinationRecords, value: country
                add_vaccination_records(monitor, value);
                // there is no need for answers
        break;

        case 3: // command: /searchVaccinationStatus, value: citizenID
                search_vaccination_status(monitor, value);
                answer();
        break;

        case 4: // command: exit, value: NULL
                travel_monitor_finalize(monitor);
        break;

        default: // error
            break;
    }
    return true;
}

void travel_monitor_destroy(TravelMonitor m) {
    monitor_manager_destroy(m->manager);
    ht_destroy(m->virus_stats);
    free(m);
}
