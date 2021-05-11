#include "Monitor.h"
#include "StructManipulation.h"

void visit(Pointer _p) {
    Person p = (Person)_p;
    printf("%s %s %s %s %d\n", p->citizenID, p->firstName, p->lastName,
           p->country_t->country, p->age);
}

bool error_flag = false;
char error_msg[BUFSIZ];
char ans_buffer[BUFSIZ];
int out_fd = -1;


// static void print_error(bool exit_fail) {
//     fprintf(stderr, "%s\n", error_msg);
//     error_flag = false;
//     memset(error_msg, 0, BUFSIZ);
//     if (exit_fail) exit(EXIT_FAILURE);
// }

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

void answer(int opcode) {
    int buf_len = strlen(ans_buffer);
    if (buf_len) {    char buf[buf_len];
        memset(buf, 0, buf_len);
        memcpy(buf, ans_buffer, buf_len);
        send_msg(out_fd, buf, buf_len, opcode);
    }
    send_msg(out_fd, NULL, 0, MSGEND_OP);

    // puts(ans_buffer); //remove

    memset(ans_buffer, 0, BUFSIZ);
}

static void vaccinate_citizen(VirusInfo v, VaccRec vacc_rec, char *date) {
    Pointer vacc_rec_dummy=NULL;
    // delete him from the non-vaccinated group
    sl_delete(v->not_vaccinated, vacc_rec, false, &vacc_rec_dummy);

    // debug
    #ifdef DEBUG
        assert(!sl_search(v->not_vaccinated, vacc_rec));
        assert(vacc_rec == vacc_rec_dummy);
    #endif

    // insert the person instance into the vaccinated group and
    sl_insert(v->vaccinated, vacc_rec, false, &vacc_rec_dummy);
    bf_insert(v->bf, vacc_rec->p->citizenID);
    // we also have to alter the date in vacc_rec
    vacc_rec->date = calloc(1 + strlen(date), sizeof(char));
    strcpy(vacc_rec->date, date);

    // debug
    #ifdef DEBUG
        assert(sl_search(v->vaccinated, vacc_rec));
    #endif
}

// Function to insert a record in the structs of the according virusInfo tuple
// If update flag is true then we will update the record if it exists
// In the right spirit this is also a vaccination operation for the citizens
static void virus_info_insert(Monitor monitor, Person p, bool update, char *virusName, char *date, bool is_vaccinated) {
    // First we will check if the virus info is created
    struct virus_info_tuple dummy = {.virusName=virusName};
    Pointer key;

    // search for the virus info in the respective HT
    if ((key = list_node_get_entry(monitor->virus_info, list_find(monitor->virus_info, &dummy)))) {
        Pointer vacc_rec = NULL, vacc_rec_dummy = NULL;
        VirusInfo v = (VirusInfo)key;
        VaccRec dummy_vr = vacc_rec_create(p, NULL, false);

        // pick the appropriate skip list to search and/or insert
        SL sl = (is_vaccinated == true) ? v->vaccinated : v->not_vaccinated;
        
        // try to insert 
        if ((vacc_rec = sl_search(v->not_vaccinated, dummy_vr))) {
            // the person is in the not-vaccinated group and he got
            // vaccinated, since there is no point to process people already
            // vaccinated and/or update based on records that refer to
            // not-vaccinated people
            if (update && is_vaccinated)
                vaccinate_citizen(v, vacc_rec, date);
            else if (!update) {
                error_flag = true;
                sprintf(error_msg, "ERROR IN RECORD %s %s %s %s %d %s %s %s", p->citizenID, p->firstName, p->lastName, p->country_t->country, p->age, virusName, is_vaccinated ? "YES":"NO", date ? date : "");
            }
        } else if (!(vacc_rec = sl_search(v->vaccinated, dummy_vr))) {
            // here the person is neither of vacc of not-vacc lists
            // so just add him
            VaccRec vr = vacc_rec_create(p, date, true);
            sl_insert(sl, vr, false, &vacc_rec_dummy);
            if (is_vaccinated)
                bf_insert(v->bf, p->citizenID);

            #ifdef DEBUG
            if (is_vaccinated)
                assert(bf_contains(v->bf, p->citizenID));
            assert(sl_search(sl, vr));
            #endif
        } else {
            // the person is in vaccinated list
            #ifdef DEBUG
            assert(sl_search(v->vaccinated, dummy_vr));
            #endif
            error_flag = true;
            if (update)
                sprintf(error_msg, "ERROR: CITIZEN %s ALREADY VACCINATED ON %s", ((VaccRec)vacc_rec)->p->citizenID, ((VaccRec)vacc_rec)->date);
            else
                sprintf(error_msg, "ERROR IN RECORD %s %s %s %s %d %s %s %s", p->citizenID, p->firstName, p->lastName, p->country_t->country, p->age, virusName, is_vaccinated ? "YES":"NO", date ? date : "");
        }
        // virus_info_destroy(dummy);
        vacc_rec_destroy(dummy_vr);
    } else {
        // there is no instance of the virusInfo, so we will insert a new rec 
        // and we will also insert the person in the bloom filter and 
        // into the appropriate skip list
        VirusInfo new_vi = virus_info_create(virusName, monitor->bloom_size, monitor->sl_height, monitor->sl_factor);
        list_insert(monitor->virus_info, new_vi, true);
        if (is_vaccinated)
            // insert into BF
            bf_insert(new_vi->bf, p->citizenID);

        //insert into the appropriate skip list
        SL sl = (is_vaccinated == true) ? new_vi->vaccinated : new_vi->not_vaccinated;
        VaccRec vr = vacc_rec_create(p, date, true);
        sl_insert(sl, vr, false, &key);
        #ifdef DEBUG
        assert(list_find(monitor->virus_info, new_vi));
        assert(sl_search(sl, vr));
        #endif
    }
}

// Function to insert an entry to  the country index of a vaccine monitor
static void country_index_insert(Monitor monitor, Person p) {
    CountryIndex dummy = p->country_t;
    Pointer key;
    // check if we already have a list
    if ((key = list_node_get_entry(monitor->citizen_lists_per_country, list_find(monitor->citizen_lists_per_country, dummy)))) {
        // there is already an index list so,
        // we just add the record into it, if it does not already exists
        CountryIndex c = (CountryIndex)key;
        if (!list_find(c->citizen_list, p)) {
            p->country_t = c; // set the country pointer to the one that exists in the hashtable
            list_insert(c->citizen_list, p, true); // insert the person in the country index
            #ifdef DEBUG
            assert(list_find(p->country_t->citizen_list, p));
            #endif
        }

        // destroy the dummy node
        country_index_destroy(dummy);
        
        // for debuging purposes
        #ifdef DEBUG
        assert(list_find(c->citizen_list, p));
        #endif
    } else {
        // there is no country index in the hash. So we must insert the dummy
        // one there and then add the record to the index list as a first entry
        // (head)
        list_insert(monitor->citizen_lists_per_country, dummy, true);
        list_insert(dummy->citizen_list, p, true);
        #ifdef DEBUG
        assert(list_find(monitor->citizen_lists_per_country, dummy));
        assert(list_find(p->country_t->citizen_list, p));
        #endif
    }
}


// Vaccine Monitor Utilities

// Insert the record into the monitor. 
// If the update flag is one then, if we find an instance of it, about a specific virus
// either ommit the incosistent record, or 
// actually update the proper virus info lists and bloom filter 
static void insert_record(Monitor monitor, char *record, bool update) {
    Person p = str_to_person(record);

    if (p) {
        Pointer old, key;
        // now we have to insert the record in the HTs
        bool exists = ht_contains(monitor->citizens, p, &key);

        // first check the general index for citizenID (if the record is inconsistent we just ignore it)
        if (!exists) {
            // This is the first occurence of the record so we insert it in the general hash
            // and the countries hash
            ht_insert(monitor->citizens, p, false, &old);
            country_index_insert(monitor, p); 
            key = p;
        }

        // now we have to insert and/or update the respective virus info
        // if the record is incosistent with the current instance in the general table then 
        // ommit the operation and fail returning the proper message
        if (exists && !person_equal(p, (Person)key)) {
            sprintf(error_msg, "ERROR IN RECORD %s", record);
            error_flag = true;
        } else {
            // key = p if it does not exists in the general citizen hash,
            // other wise key = already existing key
            virus_info_insert(monitor, key, update, p->virusName, p->date, p->vaccinated);
        }

        // if the person exists there is no reason to keep the instance we created
        if (exists) {
            person_complete_destroy(p);
        }
    } else {
        sprintf(error_msg, "ERROR IN RECORD %s", record);
        error_flag = true;
    }
}

static void insert_from_dir(Monitor monitor, FM fm, DirectoryEntry dentry) {
    char **records=NULL;
    int length=0;
    
    // get the record lines from all the files in the monitor
    fm_read_from_dir_entry(fm, dentry, &records, &length);

    // add all the records in the monitor
    for (int rec_id = 0; rec_id < length; rec_id++) {
        insert_record(monitor, records[rec_id], false);
        free(records[rec_id]);
    }
    free(records);

}

static void insert_from_fm(Monitor monitor, FM fm) {
    // get the list of added records in the file manager
    List dir_list = fm_get_directory_list(fm);
    // iterate the directories and add their records in the monitor's structs
    for (ListNode node = list_get_head(dir_list); node != NULL; node = list_get_next(dir_list, node)) {
        DirectoryEntry dentry = list_node_get_entry(dir_list, node); 
        insert_from_dir(monitor, fm, dentry);
    }
}

static void travel_request(Monitor monitor, char *value) {
    // value: citizenID date countryFrom countryTo virusName
    bool found = false;
    char **values = NULL;
    int values_len = 0;
    // parse the values
    values = parse_line(value, &values_len, FIELD_SEPARATOR);
    
    struct person dummy_p = {.citizenID=values[0]};
    Pointer rec = NULL;
    if (ht_contains(monitor->citizens, &dummy_p, &rec)) {
        // get the virus skip lists
        struct virus_info_tuple dummy_v = {.virusName=values[values_len-1]};
        VirusInfo virus_info = (VirusInfo)list_node_get_entry(monitor->virus_info, list_find(monitor->virus_info, &dummy_v));
        if (virus_info) {
            // if the country has records about the specific virus
            // check if the person is in the vaccinated skip list
            struct vaccine_record dummy_vc = {.p=(Person)rec};
            VaccRec vac_rec = (VaccRec)sl_search(virus_info->vaccinated, &dummy_vc);

            // if he is vaccinated set the answer buffer as "YES <date of vaccination>"
            if (vac_rec){
                sprintf(ans_buffer, "YES%s%s", FIELD_SEPARATOR, vac_rec->date);
                found=true;
                monitor->accepted ++; // increase the accepted counter
            }
        }
    }

    if (!found){
        monitor->rejected ++; // increase the rejected counter
        sprintf(ans_buffer, "NO");
    }

    for (int i = 0; i < values_len; i++) free(values[i]);
    free(values);
}

void monitor_send_blooms(Monitor monitor, int out_fd);
// function to search for added requests in the user-defined directory
static void add_vaccination_records(Monitor monitor, char *value) {
    // value = NULL

    // first we use the File Manager Utility to check for new files in the already assigned directories
    char **new_files = NULL;
    int num_new_files = 0;
    fm_check_for_updates(monitor->fm, &new_files, &num_new_files);
    
    #ifdef DEBUG
    printf("Found %d new file/paths.\n", num_new_files);
    #endif

    // now iterate through the new files and add their records in the monitor
    for (int i = 0; i < num_new_files; i++) {
        // get the records of the file
        char **records = NULL;
        int num_records = 0;
        fm_read_from_file(monitor->fm, new_files[i], &records, &num_records);
        // add all the records in the monitor
        for (int rec_id = 0; rec_id < num_records; rec_id++) {
            // insert the new records
            insert_record(monitor, records[rec_id], true);
            // delete the records entry since it's not useful no more
            free(records[rec_id]);
        }
        // delete the records table since it's no usefull anymore
        free(records);
        // delete the new_files[i] path since it's not useful anymore
        free(new_files[i]);
    }
    // free the memory hold by the new_files table.
    free(new_files);
    // for debug
    #ifdef DEBUG
        ht_print_keys(monitor->citizens, visit);
    #endif
    
    // Now we have to send the monitors
    monitor_send_blooms(monitor, out_fd);
}

// utility to check whether a citizen is vacc's or not for a specific virus
// input values are person struct and the virus info struct for the relative virus

static void check_for_citizen(Person citizen, VirusInfo virus_info) {
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);
    VaccRec vr = NULL;
    struct vaccine_record dummy_vr = {.p=citizen};
    if ((vr = (VaccRec)sl_search(virus_info->vaccinated, &dummy_vr))) {
        sprintf(buf, "%s, VACCINATED ON %s\n", virus_info->virusName, vr->date);
    } else if ((vr = (VaccRec)sl_search(virus_info->not_vaccinated, &dummy_vr))) {
        sprintf(buf, "%s NOT YET VACCINATED\n", virus_info->virusName);
    }

    // if we actually found the citizen in either of the lists
    if (strlen(buf))
        strcat(ans_buffer, buf);
}

static void search_vaccination_status(Monitor monitor, char *value) {
    // value: citizenID

    // first we have to get the citizen from the monitor table
    struct person dummy_p = {.citizenID=value};
    Pointer rec = NULL;
    if (ht_contains(monitor->citizens, &dummy_p, &rec)) {
        // we found the citizen
        Person citizen = (Person)rec;
        // get the basic details
        sprintf(ans_buffer, "%s %s %s %s\nAGE %d\n", citizen->citizenID, 
                                                    citizen->firstName, 
                                                    citizen->lastName, 
                                                    citizen->country_t->country,
                                                    citizen->age);
        // Now we have to check for every virus in the current vaccine monitor 
        // whether the citizen is vacc'd or non-vacc'd
        ListNode node = list_get_head(monitor->virus_info);

        while (node) {
            VirusInfo virus_info = (VirusInfo)list_node_get_entry(monitor->virus_info, node);
            check_for_citizen(citizen, virus_info);
            node = list_get_next(monitor->virus_info, node);
        }
    }
}

// utility to print the logs
static void monitor_print_logs(Monitor monitor, char *logs_path) {
    // first we have to create the file
    char logfile_path[BUFSIZ];
    memset(logfile_path, 0, BUFSIZ);
    sprintf(logfile_path, "%s/log_file.%d", logs_path, getpid());

    int log_fd = open(logfile_path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    // first we have to write the countries
    ListNode node = list_get_head(monitor->citizen_lists_per_country);
    while (node) {
        CountryIndex c = (CountryIndex)list_node_get_entry(monitor->citizen_lists_per_country, node);
        write(log_fd, c->country, strlen(c->country));
        write(log_fd, "\n", 1);
        node = list_get_next(monitor->citizen_lists_per_country, node);
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

// return the last part of path. DOES NOT ALLOCATE MEMORY
static char *extract_dir_from_path(char *path) {
    int path_len = strlen(path);
    int i;
    for (i = path_len - 1; i > 0; i--) {
        if (path[i] == '/') break;
    }
    return path + i + 1;
}

static void countries_to_buf(Monitor monitor, char **header, int *header_len) {
    *header = NULL;
    *header_len = 0;

    // get the country_paths list
    List l = fm_get_directory_list(monitor->fm);
    ListNode node = list_get_head(l);
    while (node) {
        // get the dir name
        DirectoryEntry dentry = list_node_get_entry(l, node);
        char *dir_path = fm_get_dir_name(monitor->fm, dentry);
        char *dir = extract_dir_from_path(dir_path);

        ListNode next = list_get_next(l, node);

        // update the header buffer
        int new_len = next ? (*header_len) + strlen(dir) + 1 : (*header_len) + strlen(dir);
        char *new_header = calloc(new_len, sizeof(char));
        if (*header_len) memcpy(new_header, *header, *header_len);
        memcpy(new_header+(*header_len), dir, strlen(dir));
        
        if (next)
            memcpy(new_header + (*header_len) + strlen(dir), SEP, 1);

        *header_len = new_len;
        free(*header);
        *header = new_header;

        // proceed to the next node
        node = next;
    }
}

// function to set the buffer with bloom filters we send above
static void set_bf_buf(char *header, int header_len, VirusInfo virus_info, char **buf, int *bufsiz) {
    // format <length to read till you reach bf (10 digs)><virus$header><bf_string>
    *buf = NULL;
    *bufsiz = 0;
    char *virus_name = virus_info->virusName;
    char *bloom_filter_buf = NULL;
    int bf_bufsiz = 0;

    // get the bloom filter from virus info in buffer form (format is specified by
    // the BF struct)
    bf_to_buffer(virus_info->bf, &bloom_filter_buf, &bf_bufsiz, SEP[0]);
    // Now we are ready to construct the message
    if (bf_bufsiz) {
        *bufsiz = 10 + strlen(virus_name) + 1 + header_len + bf_bufsiz;
        *buf = calloc(*bufsiz, sizeof(char));
        int bytes_till_bf = 10 + strlen(virus_name) + 1 + header_len;
        
        // set the header of the message
        sprintf(*buf, "%0*d", 10, bytes_till_bf);
        memcpy((*buf) + 10, virus_name, strlen(virus_name));
        memcpy((*buf) + 10 + strlen(virus_name), SEP, 1);
        memcpy((*buf) + 10 + strlen(virus_name) + 1, header, header_len);
        
        // set the bf filter msg into the buffer
        memcpy((*buf) + bytes_till_bf, bloom_filter_buf, bf_bufsiz);
    }

    if (bf_bufsiz) free(bloom_filter_buf);
}

// Basic Monitor Methods

void monitor_initialize(int _out_fd) { 
    // for basic workflow loop
    is_end = false;

    // for communication
    out_fd = _out_fd;

    // basic I/O globals
    error_flag = false;
    memset(ans_buffer, 0, BUFSIZ);
    memset(error_msg, 0, BUFSIZ);
}

void monitor_finalize(Monitor monitor) {
    // first we have to print logs
    monitor_print_logs(monitor, MONITOR_LOG_PATH);
    // then we have to exit the main loop
    is_end = true;
}

Monitor monitor_create(FM fm, int bloom_size, int sl_height, float sl_factor) {
    Monitor m = calloc(1, sizeof(*m));
    m->accepted = 0;
    m->rejected = 0;
    m->bloom_size = bloom_size;
    m->sl_height = sl_height;
    m->sl_factor = sl_factor;
    m->citizens = ht_create(person_cmp, person_hash, person_destroy);
    m->citizen_lists_per_country = list_create(country_index_cmp, country_index_destroy);
    m->virus_info = list_create(virus_info_cmp, virus_info_destroy);
    m->fm = fm;
    if (fm) {
        // insert all the valid records of citizens from the directories in the fm
        insert_from_fm(m, fm);
        #ifdef DEBUG
        ht_print_keys(m->citizens, visit);
        #endif
    }
    
    return m;
}

void monitor_destroy(Monitor m) {
    list_destroy(&(m->citizen_lists_per_country));
    ht_destroy(m->citizens);
    list_destroy(&(m->virus_info));
    fm_destroy(m->fm);
    free(m);
}

bool monitor_act(Monitor monitor, int expr_index, char *value) {
    switch (expr_index) {
    case 0: // command: /travelRequest, value: citizenID date, countryFrom countryTo virusName
            travel_request(monitor, value);
            answer(Q1_PAR); // response to parent for Q1
        break;
    case 2: // command: /addVaccinationRecords, value: country-dir path
            add_vaccination_records(monitor, value);
        break;

    case 3: // command: /searchVaccinationStatus, value: citizenID
            search_vaccination_status(monitor, value);
            answer(Q4_PAR); // response to parent for Q4
        break;

    case 4: // command: /exit, value: -
            monitor_finalize(monitor);
            break;
    default:
        return false;
        break;
    }
    return true;
}

void monitor_send_blooms(Monitor monitor, int out_fd) {

    // we have to send the blooms for all the viruses to the out_fd
    // send each one separetely
    ListNode node = list_get_head(monitor->virus_info);
    
    // set the countries 
    char *header = NULL;
    int header_len = 0;

    countries_to_buf(monitor, &header, &header_len);
    while (node) {
        // get the virus entry 
        VirusInfo virus_info = list_node_get_entry(monitor->virus_info, node);
        // initiate the buffer by putting a header of <virus name>$<country 1>$<country 2>$...$<country n>
        // and after that add the BF
        char *buf = NULL;
        int bufsiz = 0;
        set_bf_buf(header, header_len, virus_info, &buf, &bufsiz);
        // send the buffer
        send_msg(out_fd, buf, bufsiz, INIT_PAR);
        // free the memory
        free(buf);
        node = list_get_next(monitor->virus_info, node);
    }

    // notify that you done sending bfs
    send_msg(out_fd, NULL, 0, MSGEND_OP);

    // free the buffers used
    free(header);
}
