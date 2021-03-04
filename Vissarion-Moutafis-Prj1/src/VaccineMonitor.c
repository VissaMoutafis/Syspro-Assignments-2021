#include "VaccineMonitor.h"
#include "StructManipulation.h"

bool error_flag = false;
char error_msg[BUFSIZ];
char ans_buffer[BUFSIZ];

static void print_error(bool exit_fail) {
    fprintf(stderr, "%s\n", error_msg);
    error_flag = false;
    memset(error_msg, 0, BUFSIZ);
    if (exit_fail)
        exit(EXIT_FAILURE);
}

// all the possible commands for the tty API of the app
int pos_cmds_len = 8;

// for format checking
char *allowed_formats[] = {"vaccineStatusBloom",        "vaccineStatus",
                           "populationStatus",          "popStatusByAge",
                           "insertCitizenRecord",       "vaccinateNow",
                           "list-nonVaccinated-Persons", "exit"};

// for help printing
char *possible_commands[] = {
    "/vaccineStatusBloom citizenID virusName",
    "/vaccineStatus citizenID [virusName]",
    "/populationStatus [country] virusName date1 date2",
    "/popStatusByAge [country] virusName date1 date2",
    "/insertCitizenRecord citizenID firstName lastName jcountry age virusName "
    "YES/NO [date]",
    "/vaccinateNow citizenID firstName lastName contry age virusName",
    "/list-nonVaccinated-Persons virusName",
    "/exit"};

// for value list argument checking (swallow checks)
int max_values[] = {2, 2, 4, 4, 8, 6, 1, 0};
int min_values[] = {2, 1, 3, 3, 8, 6, 1, 0};

struct vaccine_monitor {
    // General indexing for citizens
    HT citizens;
    HT citizen_lists_per_country;
    // hash table {key: virusName, items:[bf, vacc_sl, non_vacc_sl]}
    List virus_info;
    int bloom_size;
    int sl_height;
    float sl_factor;
};

static void vaccinate_citizen(VirusInfo v, VaccRec vacc_rec, char *date) {
    Pointer vacc_rec_dummy=NULL;
    // delete him from the non-vaccinated group
    if (!sl_delete(v->not_vaccinated, vacc_rec, false, &vacc_rec_dummy)) {
        fprintf(stderr, "\nPROBLEM DURING DELETION OF %s.\n",
                vacc_rec->p->citizenID);
        exit(1);
    }

    // debug
    #ifdef DEBUG
        assert(!sl_search(v->not_vaccinated, vacc_rec));
        assert(vacc_rec == vacc_rec_dummy);
    #endif

    // insert the person instance into the vaccinated group and
    sl_insert(v->vaccinated, vacc_rec, false, &vacc_rec_dummy);
    bf_insert(v->bf, vacc_rec->p);
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
static void virus_info_insert(VaccineMonitor monitor, Person p, bool update, char *virusName, char *date, bool is_vaccinated) {
    // First we will check if the virus info is created
    VirusInfo dummy = virus_info_create(virusName, monitor->bloom_size, monitor->sl_height, monitor->sl_factor);
    Pointer key;

    // search for the virus info in the respective HT
    if ((key = list_node_get_entry(monitor->virus_info, list_find(monitor->virus_info, dummy)))) {
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
        } else if (sl == v->not_vaccinated || !(vacc_rec = sl_search(sl, dummy_vr))) {
            // at this case the record does not 

            // if we cannot find him in sl then we must try to search for the other list 
            VaccRec vr = vacc_rec_create(p, date, true);
            sl_insert(sl, vr, false, &vacc_rec_dummy);
            if (is_vaccinated)
                bf_insert(v->bf, p);
            #ifdef DEBUG
            if (is_vaccinated)
                assert(bf_contains(v->bf, p));
            assert(sl_search(sl, vr));
            #endif
        }
        virus_info_destroy(dummy);
        vacc_rec_destroy(dummy_vr);
    } else {
        // there is no instance of the virusInfo, so we will insert the dummy rec 
        // and we will also insert the person in the bloom filter and 
        // into the appropriate skip list
        list_insert(monitor->virus_info, dummy, true);
        if (is_vaccinated)
            // insert into BF
            bf_insert(dummy->bf, p);

        //insert into the appropriate skip list
        SL sl = (is_vaccinated == true) ? dummy->vaccinated : dummy->not_vaccinated;
        VaccRec vr = vacc_rec_create(p, date, true);
        sl_insert(sl, vr, false, &key);
        #ifdef DEBUG
        assert(list_find(monitor->virus_info, dummy));
        assert(sl_search(sl, vr));
        #endif
    }
}

// Function to insert an entry to  the country index of a vaccine monitor
static void country_index_insert(VaccineMonitor monitor, Person p) {
    CountryIndex dummy = p->country_t;
    Pointer key;
    // check if we already have a list
    if (ht_contains(monitor->citizen_lists_per_country, dummy, &key)) {
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
        ht_insert(monitor->citizen_lists_per_country, dummy, false, &key);
        list_insert(dummy->citizen_list, p, true);
        #ifdef DEBUG
        assert(ht_contains(monitor->citizen_lists_per_country, dummy, &key));
        assert(list_find(p->country_t->citizen_list, p));
        #endif
    }
}

// Vaccine Monitor Utilities

// create a person from a data line in the file
Person str_to_person(char *record) {
    // First we have to parse the record
    char **parsed_rec = NULL;
    int cols = -1;
    parsed_rec = parse_line(record, &cols, FIELD_SEPARATOR);
    
    // if the instance is incosistent then ommit it
    if (!check_person_constistency(parsed_rec, cols)) {
        for (int i = 0; i < cols; i++) free(parsed_rec[i]);
        free(parsed_rec);
        return NULL;
    }
    // create the person instance
    Person p = create_person(parsed_rec[0], 
                            parsed_rec[1], 
                            parsed_rec[2],
                            country_index_create(parsed_rec[3]), // insert a country index pointer
                            atoi(parsed_rec[4]), // insert age as an integer
                            parsed_rec[5],
                            parsed_rec[6], 
                            cols == 8? parsed_rec[7] : NULL, //insert the date only in case there is a 8-th element 
                            true); // make a deep copy
    
    // free the memory of the parse array
    for (int i = 0; i < cols; i++) free(parsed_rec[i]);
    free(parsed_rec);

    return p;
}

// Insert the record into the monitor. 
// If the update flag is one then, if we find an instance of it, about a specific virus
// either ommit the incosistent record, or 
// actually update the proper virus info lists and bloom filter 
static void insert_record(VaccineMonitor monitor, char *record, bool update) {
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

bool insert_from_file(VaccineMonitor monitor, char *in_filename) {
    FILE *in = fopen(in_filename, "r");
    if (!in) return false;
    // get the number of lines of the input file
    int lines = fget_lines(in_filename);

    for (unsigned int i = 0; i < lines; i++) {
        char *record_line = make_str(&in);
        insert_record(monitor, record_line, false);
        free(record_line);
        if (error_flag) {
            print_error(false);
        }
    }

    fclose(in);
    return true;
}

// Basic commands implementation
static void vaccine_status_bloom(VaccineMonitor monitor, char *value) {
    // value = citizenID, virusName
    bool vaccinated = false;
    int cols = -1;
    char **parsed_values = parse_line(value, &cols, FIELD_SEPARATOR);
    // create a dummy struct for searching
    Person dummy_p = create_person(parsed_values[0], NULL, NULL, NULL, 0, NULL, "NO", NULL, false);

    // find the actual person in the database
    Pointer person;
    if (ht_contains(monitor->citizens, dummy_p, &person)) {
        Pointer v;
        // Since the person is actually in the database then
        VirusInfo dummy_v = virus_info_create(parsed_values[1], BF_HASH_FUNC_COUNT, 1, 0.0);
        if ((v = list_node_get_entry(monitor->virus_info, list_find(monitor->virus_info, dummy_v))))
            vaccinated = bf_contains(((VirusInfo)v)->bf, person);

        virus_info_destroy(dummy_v);
    }

    // free the allocated memory
    free(dummy_p);
    for (int i = 0; i < cols; i++) free(parsed_values[i]);
    free(parsed_values);

    // change the answer buffer accordingly
    sprintf(ans_buffer, "%s", vaccinated ? "MAYBE" : "NOT VACCINATED");
}

static void vaccine_status(VaccineMonitor monitor, char *value) {
    // value = citizenID, virusName
    int cols = -1;
    char **parsed_values = parse_line(value, &cols, FIELD_SEPARATOR);

    // create a dummy struct for searching
    Person dummy_p = create_person(parsed_values[0], NULL, NULL, NULL, 0, NULL, "NO", NULL, false);

    // find the actual person in the database
    Pointer person;
    if (ht_contains(monitor->citizens, dummy_p, &person)) {
        VaccRec dummy_vr = vacc_rec_create(person, NULL, false);
        Pointer vp;
        if (cols == 2) {
            // Since the person is actually in the database then
            VirusInfo dummy_v = virus_info_create(parsed_values[1], BF_HASH_FUNC_COUNT, 1, 0.0);
            if ((vp = list_node_get_entry(monitor->virus_info, list_find(monitor->virus_info, dummy_v)))) {
                VirusInfo v =(VirusInfo)vp;
                Pointer key;
                if ((key = sl_search(v->vaccinated, dummy_vr)))
                    sprintf(ans_buffer, "%s YES %s", v->virusName, ((VaccRec)key)->date);
                else
                    sprintf(ans_buffer, "%s NO", v->virusName);
            } else 
                sprintf(ans_buffer, "%s IS NOT A REGISTERED VIRUS\n", parsed_values[1]);

            virus_info_destroy(dummy_v);
        } else if (cols == 1) {
            ListNode node = list_get_head(monitor->virus_info);
         
            // search every node in the virus info list
            while (node) {
                VirusInfo v = list_node_get_entry(monitor->virus_info, node);
                strcat(ans_buffer, v->virusName);
                Pointer key;                

                // if the citizen is part of the vaccinated people 
                if ((key = sl_search(v->vaccinated, dummy_vr))) {
                    char buf[100];
                    memset(buf, 0, 100);
                    sprintf(buf, " YES %s\n", ((VaccRec)key)->date);
                    strcat(ans_buffer, buf);
                } else // if he is not
                    strcat(ans_buffer, " NO\n");
                // proceed to the next node 
                node = list_get_next(monitor->virus_info, node);
            }
        }
        free(dummy_vr);
    } else 
        strcpy(ans_buffer, "ERROR: CITIZEN ID DOES NOT EXIST");
    // free the allocated memory
    free(dummy_p);
    for (int i = 0; i < cols; i++) free(parsed_values[i]);
    free(parsed_values);
}

// Basic Vaccine Monitor Methods

void vaccine_monitor_initialize(void) { 
    is_end = false;
    memset(ans_buffer, 0, BUFSIZ);
    memset(error_msg, 0, BUFSIZ);
}

void vaccine_monitor_finalize(void) {
    is_end = true;
}

void visit(Pointer _p) {
    Person p = (Person)_p;
    printf("%s %s %s %s %d\n", p->citizenID, p->firstName, p->lastName, p->country_t->country, p->age);
}

VaccineMonitor vaccine_monitor_create(char *input_filename, int bloom_size, int sl_height, float sl_factor) {
    VaccineMonitor m = calloc(1, sizeof(*m));

    m->bloom_size = bloom_size;
    m->sl_height = sl_height;
    m->sl_factor = sl_factor;
    m->citizens = ht_create(person_cmp, person_hash, person_destroy);
    m->citizen_lists_per_country = ht_create(country_index_cmp, country_index_hash, country_index_destroy);
    m->virus_info = list_create(virus_info_cmp, virus_info_destroy);

    if (input_filename) {
        // insert all the valid records of citizens from this file
        printf("Inserting from file '%s' ...\n", input_filename);
        insert_from_file(m, input_filename);
        #ifdef DEBUG
        // ht_print_keys(m->citizens, visit);
        #endif
    }

    return m;
}

void vaccine_monitor_destroy(VaccineMonitor m) {
    ht_destroy(m->citizen_lists_per_country);
    ht_destroy(m->citizens);
    list_destroy(&(m->virus_info));
    free(m);
}

bool vaccine_monitor_act(VaccineMonitor monitor, int expr_index, char *value) {
    switch (expr_index) {
    case 0: // command = vaccineStatusBloom , value = "citizenID virusName"
            vaccine_status_bloom(monitor, value);
            printf("%s\n", ans_buffer);
            memset(ans_buffer, 0, BUFSIZ);
        break;

    case 1: // command = vaccineStatus, values = citizenID [virusName]
            vaccine_status(monitor, value);
            printf("%s\n", ans_buffer);
            memset(ans_buffer, 0, BUFSIZ);
        break;

    case 2:

        break;

    case 3:

        break;

    case 4:

        break;

    case 5:

        break;

    case 6:

        break;

    case 7:
        // command: exit, value: NULL
        vaccine_monitor_finalize();
        break;

    default:
        return false;
        break;
    }
    return true;
}