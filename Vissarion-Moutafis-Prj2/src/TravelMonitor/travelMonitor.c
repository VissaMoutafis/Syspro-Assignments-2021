/**
*	Syspro Project 2
*	 Written By Vissarion Moutafis sdi1800119
**/
 
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
                           "/exit"};

// for help printing
char *possible_commands[] = {
    "/travelRequest citizenID date countryFrom [countryTo] virusName",
    "/travelStats virusName [date1 date2] [country]",
    "/addVaccinationRecords country",
    "/searchVaccinationStatus citizenID",
    "/exit"};

// for value list argument checking (swallow checks)
int max_values[5] = {5, 4, 1, 1, 0};
int min_values[5] = {5, 1, 1, 1, 0};


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
    m2 += (y2-y1)*12;
    return m2 - m1 < 6 && m2 >= m1;
}

static bool try_answer_travel_request(TravelMonitor monitor, void *args[], void *ret_args[]) {
    // args: {VirusStats vs, BFTuple bft, char *citizenID, char *date, char *countryTo}
    // ret_args: NULL
    VirusStats vs = (VirusStats)args[0];
    BFTuple bft = (BFTuple)args[1];
    char *citizenID = (char *)args[2];
    char *date = (char *)args[3];
    char *countryTo = (char *)args[4];
    if (check_date(date) && !bf_contains(bft->bf, citizenID)) {
        // set up the answer
        memset(ans_buffer, 0, BUFSIZ);
        sprintf(ans_buffer, "REQUEST REJECTED - YOU ARE NOT VACCINATED");
        // add a rejection record
        RequestRec new_rec = request_record_create(date, countryTo);
        list_insert(vs->rejected, new_rec, true);
        monitor->rejected++;
        return true;
    }

    return false;
}

static void send_query_to_child(int opcode, int expr_id, char *value, int out_fd, u_int32_t bufsiz) {
    int buf_len = strlen(value) + 10;  // value + 10 digs for expr index
    char buf[buf_len];
    sprintf(buf, "%0*d", 10, expr_id);
    memcpy(buf + 10, value, buf_len - 10);
    send_msg(out_fd, bufsiz, buf, buf_len, Q1_CHLD);
    send_msg(out_fd, bufsiz, NULL, 0, MSGEND_OP);
}

static bool delegate_travel_request(TravelMonitor monitor, void *args[], void *ret_args[]) {
    // args: {vs, value, countryFrom, date, bft}
    // ret_args: NULL
    VirusStats vs = (VirusStats)args[0]; 
    char *value = (char *)args[1]; 
    char *countryFrom = (char *)args[2]; 
    char *req_date = (char *)args[3];
    char *countryTo = (char *)args[4];

    // first find the respective monitor
    struct trace t = {.country = countryFrom};
    Pointer entry = NULL;
    if (check_date(req_date) && ht_contains(monitor->manager->countries_index, &t, &entry)) {
        MonitorTrace *m_trace = ((Trace)entry)->m_trace;

        // send a message and w8 for response
        send_query_to_child(Q1_CHLD, 0, value, m_trace->out_fifo, monitor->buffer_size);

        // responses format: YES$<date> or NO
        char *response = NULL;
        char *date = NULL;
        void *ret_args[] = {&response, &date};
        // get response
        travel_monitor_get_response(monitor->buffer_size, monitor, travel_request_handler, m_trace->in_fifo, ret_args);

        bool accepted = false;
        // now we need to check response and set up the ans_buffer
        if (strcmp(response, "NO") == 0) {
            memset(ans_buffer, 0, BUFSIZ);
            sprintf(ans_buffer, "REQUEST REJECTED - YOU ARE NOT VACCINATED");
            monitor->rejected++;
        } else if (!check_vacc_date(date, req_date)) {
            memset(ans_buffer, 0, BUFSIZ);
            sprintf(ans_buffer, "REQUEST REJECTED - YOU WILL NEED ANOTHER VACCINATION BEFORE");
            monitor->rejected++;

        } else {
            accepted = true;
            memset(ans_buffer, 0, BUFSIZ);
            sprintf(ans_buffer, "REQUEST ACCEPTED - HAPPY TRAVELS");
            monitor->accepted++;
        }
        // add the request into the appropriate l
        List l = accepted ? vs->accepted : vs->rejected;
        list_insert(l, request_record_create(req_date, countryTo), true);

        if (date) free(date);
        free(response);
        return true;
    }

    return false;
}


static bool delegate_vacc_status_search(TravelMonitor monitor, void *args[], void *ret_args[]) {
    char *value = (char *)args[0];
    // now we have to send it to all of the monitors
    for (int i = 0; i < monitor->manager->num_monitors; i++) {
        MonitorTrace *m_trace = &(monitor->manager->monitors[i]);
        send_query_to_child(Q4_CHLD, 3, value, m_trace->out_fifo, monitor->buffer_size);
    }
    return true;
}

bool try_answer_request(TravelMonitor monitor, int opcode, void *args[], void *ret_args[]) {
    // GRAND TODO
    switch (opcode)
    {
    case 0: // command: /travelRequest
        return try_answer_travel_request(monitor, args, ret_args);
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

    case 3:
        delegate_vacc_status_search(monitor, args, ret_args);
    break;
    
    default:
        puts("Uknown activity");
    break;
    }

    return false;
}

void travel_request(TravelMonitor monitor, char *value) {
    // value: citizenID date countryFrom countryTo virusName

    bool answered = false;

    int cols = 0;
    char **parsed_value = parse_line(value, &cols, FIELD_SEPARATOR);
    char *virusName = parsed_value[cols-1];
    char *citizenID = parsed_value[0];
    char *date = parsed_value[1];
    char *countryFrom = parsed_value[2];
    char *countryTo = parsed_value[3];

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
            void *args1[] = {vs, bft, citizenID, date, countryTo};
            void *args2[] = {vs, value, countryFrom, date, countryTo};
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

// if you don't care about the country then just pass null 
// and every record regardless the country will be included
static int count_recs(List recs, char *date1, char *date2, char *country) {
    ListNode node = list_get_head(recs);
    int cnt = 0;

    while (node) {
        RequestRec r = (RequestRec)list_node_get_entry(recs, node);
        if (!country || strcmp(country, r->countryTo) == 0) {
            assert(r->date);
            if (check_date_in_range(r->date, date1, date2))
                cnt++;
        }

        node = list_get_next(recs, node);
    }

    return cnt;
}

void travel_stats(TravelMonitor monitor, char *value) {
    // values: virusName date1 date2 [country]
    int cols = 0;
    char **parsed_value = parse_line(value, &cols, FIELD_SEPARATOR);
    #ifdef DEBUG
        assert(cols);
        assert(parsed_value);
    #endif
    // get the needed fields
    char *virus_name = parsed_value[0];
    char *country = (cols==2 || cols == 4) ? parsed_value[cols-1] : NULL;
    char *date1 = "1-1-1";     
    char *date2 = "30-12-3000";

    if (cols >= 3) {
        if (check_date(parsed_value[1]) && check_date(parsed_value[2]) && dates_cmp(parsed_value[1], parsed_value[2]) < 0) {
            date1 = parsed_value[1];
            date2 = parsed_value[2];
        } else {
            error_flag = true;
            sprintf(error_msg, "ERROR: WRONG DATE FORMAT");
        }
    }

    if (!error_flag) {
        // Now we have to find the VirusStats record
        struct virus_stats dummy_vs = {.virus_name = virus_name};
        Pointer entry = NULL;
        if (ht_contains(monitor->virus_stats, &dummy_vs, &entry)) {
            VirusStats vs = (VirusStats)entry;
            int accepted = count_recs(vs->accepted, date1, date2, country);
            int rejected = count_recs(vs->rejected, date1, date2, country);
            sprintf(ans_buffer, "TOTAL REQUESTS %d\nACCEPTED %d\nREJECTED %d", accepted+rejected, accepted, rejected);
        } else {
            // we did not found the virus so there is a problem
            error_flag = true;
            sprintf(error_msg, "ERROR: VIRUS NOT RECORDED");
        }
    }

    for (int i = 0; i < cols; i++) free(parsed_value[i]);
    free(parsed_value);
}

void add_vaccination_records(TravelMonitor monitor, char *value) {
    // value = country
    assert(value);

    // Based on the country we must search for 
    // the respective monitor and send him a SIGUSR1
    char *country = value;
    struct trace dummy_trace = {.country=country};
    Pointer entry = NULL;
    if (ht_contains(monitor->manager->countries_index, &dummy_trace, &entry)) {
        // send the USR1
        MonitorTrace *m_trace = ((Trace)entry)->m_trace;
        kill(m_trace->pid, SIGUSR1);
        travel_monitor_get_response(monitor->buffer_size, monitor, get_bf_from_child, m_trace->in_fifo, NULL);

    } else {
        error_flag = true;
        sprintf(error_msg, "ERROR: '%s' is not a recorded country\n", country);
    }
}

void search_vaccination_status(TravelMonitor monitor, char *value) {
    void *args[] = {value};
    delegate_to_children(monitor, 3, args, NULL);

    char *vaccination_recs=NULL;
    void *ret_args[] = {&vaccination_recs};
    travel_monitor_get_response(monitor->buffer_size, monitor, get_vaccination_status, -1, ret_args);
    if (vaccination_recs) {
        strcpy(ans_buffer, vaccination_recs);
        free(vaccination_recs);
    }
}



void travel_monitor_restore_children(TravelMonitor monitor) {
    // Need to check all monitors for there might be more than one that failed
    for (int i = 0; i < monitor->manager->num_monitors; i++) {
        int status = -1;
        MonitorTrace m_trace;
        memset(&m_trace, 0, sizeof(m_trace));
        monitor_manager_get_at(monitor->manager, i, &m_trace);
        int pid = m_trace.pid;
        int options = WNOHANG;
        int ret;
        // get the waitpid of the child
        // ignore signal interruptions except sigint and sigquit
        while ((ret = waitpid(pid, &status, options)) == EINTR && !sigint_set && !sigquit_set);   
    
        // if the waitpid failed go to the next child
        if (ret != pid) continue;

        // printf("Found: pid: %d, status: %d\n", pid, status);
        
        // first we have to clean the monitor fifos and set pid to -1
        monitor->manager->monitors[i].pid = -1;
        monitor->manager->monitors[i].in_fifo = -1;
        monitor->manager->monitors[i].out_fifo = -1;
        // now we create a new monitor
        create_monitor(monitor, true, i);
        monitor_manager_get_at(monitor->manager, i, &m_trace);

        // send init stats
        send_init_stats_to_monitor(monitor, &m_trace);
        // and we send the countries and wait for the BFs
        send_dirs_to_monitor(monitor, &m_trace);
        // wait for the new BFS
        travel_monitor_get_response(monitor->buffer_size, monitor, get_bf_from_child, m_trace.in_fifo, NULL);

        // send an syn and wait for ack (confirm child initialization)
        bool ack_received = false;
        void *ret_args[] = {&ack_received};
        send_msg(m_trace.out_fifo, monitor->buffer_size, NULL, 0, SYN_OP);
        travel_monitor_get_response(monitor->buffer_size, monitor, accept_ack, m_trace.in_fifo, ret_args);
        if (!ack_received) {
            printf("process %d is not ready.\n", m_trace.pid);
        }
    }
}

// utility to print the logs
static void travel_monitor_print_logs(TravelMonitor monitor, char *logs_path) {
    // first we have to create the file
    char logfile_path[BUFSIZ];
    memset(logfile_path, 0, BUFSIZ);
    sprintf(logfile_path, "%s/log_file.%d", logs_path, getpid());

    int log_fd = open(logfile_path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    // first we have to write the countries
    // that exist in every monitor trace
    for (int m_id = 0; m_id < monitor->manager->num_monitors; m_id++) {
        // get the country paths table
        char **countries_paths = monitor->manager->monitors[m_id].countries_paths;
        // get the countries num
        int num_countries = monitor->manager->monitors[m_id].num_countries;
        for (int i = 0; i < num_countries; i++) {
            // take care that this returns a pointer not malloc'd string
            char *country = get_elem_name(countries_paths[i]);
            // write and dont get interrupted by a signal
            while(write(log_fd, country, strlen(country)) == -1 && errno == EINTR)errno = 0;
            while(write(log_fd, "\n", 1) == -1 && errno == EINTR)errno = 0;
        }
    }   

    // Now we have to print the Stats
    char stats[BUFSIZ];
    memset(stats, 0, BUFSIZ);
    sprintf(stats, "TOTAL TRAVEL REQUESTS %d\nACCEPTED %d\nREJECTED %d\n", 
            monitor->accepted+monitor->rejected,
            monitor->accepted,
            monitor->rejected);
        
    // write the buffer into the logfile
    write(log_fd, stats, strlen(stats)); 

    close(log_fd);
}

// Travel Monitor Routines

void travel_monitor_initialize(void) {
    error_flag = false;
    memset(error_msg, 0, BUFSIZ);
    memset(ans_buffer, 0, BUFSIZ);
    is_end = false;
}

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


void travel_monitor_finalize(TravelMonitor monitor) {
    is_end = true;
    clean_fifos();
    // send sigkill to all monitors

    for (int i = 0; i < monitor->manager->num_monitors; i++) {
        int pid = monitor->manager->monitors[i].pid;
        kill(pid, SIGKILL);
    }

    // wait for all children
    int status;
    int ret;
    errno = 0;
    while ((ret=wait(&status)) != -1)
        ;
    
    // if the error is anything else but "no child processes" then print it
    if (errno && errno != 10) {perror("wait");}

    // write logs for travel monitor
    travel_monitor_print_logs(monitor, TRAVEL_MONITOR_LOG_PATH);
}

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
            return false;
            break;
    }
    return true;
}

void travel_monitor_destroy(TravelMonitor m) {
    monitor_manager_destroy(m->manager);
    ht_destroy(m->virus_stats);
    free(m);
}
