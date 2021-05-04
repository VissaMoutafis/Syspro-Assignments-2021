#include "TravelMonitor.h"
#include "StructManipulation.h"
#include "Setup.h"

bool error_flag = false;
char error_msg[BUFSIZ];
char ans_buffer[BUFSIZ];

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


static void answer(void) {
    puts(ans_buffer);
    memset(ans_buffer, 0, BUFSIZ);
}

static void print_error(bool exit_fail) {
    fprintf(stderr, "%s\n", error_msg);
    error_flag = false;
    memset(error_msg, 0, BUFSIZ);
    if (exit_fail) exit(EXIT_FAILURE);
}

// Basic Utilities

static bool try_answer_request(TravelMonitor monitor, VirusStats vs, BFTuple bft, char * citizenID, char *countryFrom, char *date) {
    if (!bf_contains(bft->bf, citizenID)) {
        // set up the answer
        memset(ans_buffer, 0, BUFSIZ);
        sprintf(ans_buffer, "REQUEST REJECTED - YOU ARE NOT VACCINATED");

        // add a rejection record
        RequestRec new_rec = request_record_create(date);
        list_insert(vs->rejected, new_rec, true);
        return true;
    }

    return false;
}

static bool check_vacc_date(char *date, char *req_date) {
    
}

static bool delegate_to_child(TravelMonitor monitor, VirusStats vs, char *value, char *countryFrom, char *req_date) {
        // first find the respective monitor
        struct trace t = {.country = countryFrom};
        Pointer entry = NULL;
        if (ht_contains(monitor->manager->countries_index, &t, &entry)) {
            MonitorTrace* m_trace = ((Trace)entry)->m_trace, dummy;
            int id = -1;
            id = monitor_manager_search_pid(monitor->manager, m_trace->pid, &dummy);
            // send a message and w8 for response
            int buf_len = strlen(value);
            char buf[buf_len];
            memcpy(buf, value, buf_len);
            send_msg(m_trace->out_fifo, buf, buf_len, Q1_CHLD);

            // responses format: YES$<date> or NO
            char *response = NULL;
            char *date = NULL;
            void *ret_args[] = {&response, &date};
            // get response 
            travel_monitor_get_response(monitor->buffer_size, monitor, travel_request_handler, id, m_trace->in_fifo, ret_args);
            
            bool accepted = false;
            // now we need to check response and set up the ans_buffer
            if (strcmp(response, "NO")) {
                memset(ans_buffer, 0, BUFSIZ);
                sprintf(ans_buffer, "REQUEST REJECTED - YOU ARE NOT VACCINATED");
            } else if (check_vacc_date(date, req_date) > 0) {
                memset(ans_buffer, 0, BUFSIZ);
                sprintf(ans_buffer, "REQUEST REJECTED - YOU WILL NEED ANOTHER VACCINATION BEFORE");
            } else {
                accepted = true;
                memset(ans_buffer, 0, BUFSIZ);
                sprintf(ans_buffer, "REQUEST ACCEPTED - HAPPY TRAVELS");
            }
            // add the request into the appropriate l
            List l = accepted ? vs->accepted : vs->rejected;
            list_insert(l, request_record_create(date), true);
            return true;
        }

        return false;
}

void travel_request(TravelMonitor monitor, char *value) {
    // value: citizenID date countryFrom [countryTo] virusName

    bool answered = false;

    int cols = 0;
    char **parsed_value = parse_line(value, &cols, FIELD_SEPARATOR);
    char *virusName = parsed_value[cols-1];
    char *citizenID = parsed_value[0];
    char *date = parsed_value[1];
    char *countryFrom = parsed_value[2];

    // if we cannot answer the request here (bf returns true)
    // we delegate to the appropriate process
    struct virus_stats dummy_vs = {.virus_name = virusName};
    Pointer entry = NULL;
    if (ht_contains(monitor->virus_stats, &dummy_vs, &entry)) {
        // the virus is recorded so we search for country
        VirusStats vs = (VirusStats)entry;
        
        // try to locate the country
        struct bf_tuple dummy_bft = {.country=countryFrom};
        BFTuple bft = (BFTuple)sl_search(vs->bf_per_countries, &dummy_bft);
        
        // check if the parent can answer the request with certainty (ONLY IN REJECTED)
        if (bft) {
            if (!try_answer_request(monitor, vs, bft, citizenID, countryFrom, date))
                answered = delegate_to_child(monitor, vs, value, countryFrom, date);
        }
    }

    for (int i = 0; i < cols; i++) free(parsed_value[i]);
    free(parsed_value);

    if (!answered) {
        do some error stuff
    }
}

void travel_stats(TravelMonitor monitor, char *values) {

}

void add_vaccination_records(TravelMonitor monitor, char *value) {

}

void search_vaccination_status(TravelMonitor monitor, char *value) {

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

void travel_monitor_finalize(TravelMonitor monitor) {}

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
