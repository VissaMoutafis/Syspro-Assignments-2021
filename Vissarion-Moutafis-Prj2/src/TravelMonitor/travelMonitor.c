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

// specific vaccination date check
static bool check_vacc_date(char *date, char *req_date) {
    int d1, d2, m1, m2, y1, y2;
    sscanf(date, "%d-%d-%d", &d1, &m1, &y1);
    sscanf(req_date, "%d-%d-%d", &d2, &m2, &y2);

    return m2 - m1 < 6 && m2 >= m1;
}

static bool try_answer_travel_request(TravelMonitor monitor, void *args[], void *ret_args[]) {
    // args: {VirusStats vs, BFTuple bft, char *citizenID, char *date}
    // ret_args: NULL
    VirusStats vs = (VirusStats)args[0];
    BFTuple bft = (BFTuple)args[1];
    char *citizenID = (char *)args[2];
    char *date = (char *)args[3];
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



static bool delegate_travel_request(TravelMonitor monitor, void *args[], void *ret_args[]) {
    // args: {vs, value, countryFrom, date}
    // ret_args: NULL
    VirusStats vs = (VirusStats)args[0]; 
    char *value = (char *)args[1]; 
    char *countryFrom = (char *)args[2]; 
    char *req_date = (char *)args[3];
    
    // first find the respective monitor
    struct trace t = {.country = countryFrom};
    Pointer entry = NULL;
    if (ht_contains(monitor->manager->countries_index, &t, &entry)) {
        MonitorTrace *m_trace = ((Trace)entry)->m_trace, dummy;
        int id = -1;
        id = monitor_manager_search_pid(monitor->manager, m_trace->pid, &dummy);
        // send a message and w8 for response
        int buf_len = strlen(value)+10; //value + 10 digs for expr index
        char buf[buf_len];
        sprintf(buf, "%0*d", 10, 0);
        memcpy(buf+10, value, buf_len-10);
        send_msg(m_trace->out_fifo, buf, buf_len, Q1_CHLD);
        send_msg(m_trace->out_fifo, NULL, 0, MSGEND_OP);

        // responses format: YES$<date> or NO
        char *response = NULL;
        char *date = NULL;
        void *ret_args[] = {&response, &date};
        // get response
        travel_monitor_get_response(monitor->buffer_size, monitor, travel_request_handler, id, m_trace->in_fifo, ret_args);

        bool accepted = false;
        // now we need to check response and set up the ans_buffer
        if (strcmp(response, "NO") == 0) {
            memset(ans_buffer, 0, BUFSIZ);
            sprintf(ans_buffer, "REQUEST REJECTED - YOU ARE NOT VACCINATED");
        } else if (!check_vacc_date(date, req_date)) {
            memset(ans_buffer, 0, BUFSIZ);
            sprintf(
                ans_buffer,
                "REQUEST REJECTED - YOU WILL NEED ANOTHER VACCINATION BEFORE");
        } else {
            accepted = true;
            memset(ans_buffer, 0, BUFSIZ);
            sprintf(ans_buffer, "REQUEST ACCEPTED - HAPPY TRAVELS");
        }
        // add the request into the appropriate l
        List l = accepted ? vs->accepted : vs->rejected;
        list_insert(l, request_record_create(req_date), true);

        if (date) free(date);
        free(response);
        return true;
    }

    return false;
}

bool try_answer_request(TravelMonitor monitor, int opcode, void *args[], void *ret_args[]) {
    // GRAND TODO
    switch (opcode)
    {
    case 0: // command: /travelRequest
        return try_answer_travel_request(monitor, args, ret_args);
    break;

    case 1:

    break;

    case 3:
    
    break;
    
    default:
        puts("Uknown activity");
    break;
    }
    return false;
}

bool delegate_to_children(TravelMonitor monitor, int opcode, void *args[], void *ret_args[]) {
    // GRAND TODO
    switch (opcode)
    {
    case 0:
        return delegate_travel_request(monitor, args, ret_args);
    break;

    case 1:
    break;

    case 3:
    break;
    
    default:
        break;
    }
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
            void *args1[] = {vs, bft, citizenID, date};
            void *args2[] = {vs, value, countryFrom, date};
            if (!(answered = try_answer_request(monitor, 0, args1, NULL)))
                answered = delegate_to_children(monitor, 0, args2, NULL);
        }
    }

    for (int i = 0; i < cols; i++) free(parsed_value[i]);
    free(parsed_value);

    if (!answered) {
        error_flag = true;
        sprintf(error_msg, "ERROR");
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
                if (error_flag)
                    print_error(false);
                else
                    answer();
        break;

        case 1: // command: /travelStats, value: virusName date1 date2 [country]
                travel_stats(monitor, value);
                if (error_flag)
                    print_error(false);
                else
                    answer();
                break;

        case 2: // command: /addVaccinationRecords, value: country
                add_vaccination_records(monitor, value);
                if (error_flag)
                    print_error(false);
                // there is no need for answers
        break;

        case 3: // command: /searchVaccinationStatus, value: citizenID
                search_vaccination_status(monitor, value);
                if (error_flag)
                    print_error(false);
                else
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
