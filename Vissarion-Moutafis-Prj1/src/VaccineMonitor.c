/*
** Source code for the vaccine monitor app
** Implemented by Vissarion Moutafis sdi1800119
*/

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
    "/insertCitizenRecord citizenID firstName lastName country age virusName "
    "YES/NO [date]",
    "/vaccinateNow citizenID firstName lastName contry age virusName",
    "/list-nonVaccinated-Persons virusName",
    "/exit"};

// for value list argument checking (swallow checks)
int max_values[] = {2, 2, 4, 4, 8, 6, 1, 0};
int min_values[] = {2, 1, 1, 1, 7, 6, 1, 0};

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
        } else if (!(vacc_rec = sl_search(v->vaccinated, dummy_vr))) {
            // here the person is neither of vacc of not-vacc lists
            // so just add him
            VaccRec vr = vacc_rec_create(p, date, true);
            sl_insert(sl, vr, false, &vacc_rec_dummy);
            if (is_vaccinated)
                bf_insert(v->bf, p);

            #ifdef DEBUG
            if (is_vaccinated)
                assert(bf_contains(v->bf, p));
            assert(sl_search(sl, vr));
            #endif
        } else {
            // the person is in vaccinated list
            #ifdef DEBUG
            assert(sl_search(v->vaccinated, dummy_vr));
            #endif
            error_flag = true;
            sprintf(error_msg, "ERROR: CITIZEN %s ALREADY VACCINATED ON %s", ((VaccRec)vacc_rec)->p->citizenID, ((VaccRec)vacc_rec)->date);
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

static bool insert_from_file(VaccineMonitor monitor, char *in_filename) {
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
        else {
            error_flag = true;
            sprintf(error_msg, "ERROR: %s IS NOT A REGISTERED VIRUS", dummy_v->virusName);
        }
        virus_info_destroy(dummy_v);
    } else {
        error_flag = true;
        sprintf(error_msg, "ERROR: %s IS NOT A REGISTERED CITIZEN", dummy_p->citizenID);
    }

    // free the allocated memory
    free(dummy_p);
    for (int i = 0; i < cols; i++) free(parsed_values[i]);
    free(parsed_values);

    // change the answer buffer accordingly
    sprintf(ans_buffer, "%s", vaccinated ? "MAYBE" : "NOT VACCINATED");
}
static void print_virus_status(VirusInfo v, VaccRec vr, bool single_virus) {
    Pointer key;

    // if the citizen is part of the vaccinated people
    if ((key = sl_search(v->vaccinated, vr))) {
        if (!single_virus){
            char buf[100];
            memset(buf, 0, 100);
            sprintf(buf, "%s YES %s\n", v->virusName, ((VaccRec)key)->date);
            strcat(ans_buffer, buf);
        } else {
            sprintf(ans_buffer, "VACCINATED ON %s", ((VaccRec)key)->date);
        }
    } else  {
        if (!single_virus) {
            if (sl_search(v->not_vaccinated, vr)) {
                char b[BUFSIZ];
                memset(b, 0, BUFSIZ);
                sprintf(b, "%s NO\n", v->virusName);
                strcat(ans_buffer, b);
            }
        }else 
            sprintf(ans_buffer, "NOT VACCINATED");
    }
}
static void vaccine_status(VaccineMonitor monitor, char *value) {
    // value = citizenID, [virusName]
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
                print_virus_status((VirusInfo)vp, dummy_vr, true);
            } else {
                sprintf(error_msg, "ERROR: %s IS NOT A REGISTERED VIRUS\n", parsed_values[1]);
                error_flag = true;
            }
            virus_info_destroy(dummy_v);
        } else if (cols == 1) {
            ListNode node = list_get_head(monitor->virus_info);
         
            // search every node in the virus info list
            while (node) {
                VirusInfo v = list_node_get_entry(monitor->virus_info, node);
                print_virus_status(v, dummy_vr, false);
                // proceed to the next node 
                node = list_get_next(monitor->virus_info, node);
            }
        }
        free(dummy_vr);
    } else {
        sprintf(error_msg, "ERROR: %s IS NOT A REGISTERED CITIZEN ID", parsed_values[0]);
        error_flag = true;
    }
    // free the allocated memory
    free(dummy_p);
    for (int i = 0; i < cols; i++) free(parsed_values[i]);
    free(parsed_values);
}

// helper function
static void population_status_in_country (CountryIndex c, SL vaccinated, SL not_vaccinated, char *date1, char *date2,  u_int32_t vaccd_by_age[], u_int32_t age_cnt[]) {
    ListNode node = list_get_head(c->citizen_list);
    while (node) {
        // we have to check if this person is in the vaccinated skip list of the
        Person p = list_node_get_entry(c->citizen_list, node);
        VaccRec vr, dummy_vr = vacc_rec_create(p, NULL, false);
        #ifdef DEBUG
        assert(p->age >= 0);
        #endif
        int i = -1;
        if (p->age < 20)
            i = 0;
        else if (p->age < 40)
            i = 1;
        else if (p->age < 60)
            i = 2;
        else
            i = 3;
        assert(i > -1);
        
        // if we find the citizen into the vaccinated list then increase the vaccd count
        if ((vr = sl_search(vaccinated, dummy_vr)) && check_date_in_range(vr->date, date1, date2))
            vaccd_by_age[i] += 1;
        // we will count the person in the denominator (total persons vacc's/non-vacc'd)
        // only if they have a record relative to the virus in the respective skip lists
        if (vr || sl_search(not_vaccinated, dummy_vr))
            age_cnt[i] += 1;
        
        vacc_rec_destroy(dummy_vr);

        // proceed to the next node
        node = list_get_next(c->citizen_list, node);
    }
}

// beutify the printing of the status. Provide a flag "by_age" to print them accordingly
static void print_status(char *country, bool by_age, u_int32_t vaccd_by_age[], u_int32_t age_cnt[]) {
    char buf[BUFSIZ];
    memset(buf, 0, BUFSIZ);
    if (by_age) {
        sprintf(buf, "%s\n", country);
        strcat(ans_buffer, buf);
        for (int i = 0; i < 4; i++) {
            // fix the denominator
            if (vaccd_by_age[i] == 0 && age_cnt[i] == 0) age_cnt[i] = 1;
            if (i < 3)
                sprintf(buf, "%d-%d %u %.2f%%\n", 20*i, 20*(i+1), vaccd_by_age[i], 100.0*((float)vaccd_by_age[i]) / ((float)age_cnt[i]));
            else 
                sprintf(buf, "%d+ %u %.2f%%\n", 20*i, vaccd_by_age[i], 100.0*((float)vaccd_by_age[i]) / ((float)age_cnt[i]));
                
            strcat(ans_buffer, buf);
        }
        strcat(ans_buffer, "\n");
        #ifdef DEBUG
        assert(strlen(ans_buffer) < BUFSIZ);
        #endif
    } else {
        float vaccd = 0.0;
        float citizens = 0.0;
        for (int i = 0; i < 4; i++) {
            vaccd += (float)vaccd_by_age[i];
            citizens += (float)age_cnt[i];
        }
        sprintf(buf, "%s %.0f %.2f%%\n", country, vaccd, citizens ? 100.0 * vaccd / citizens : 0.0);
        strcat(ans_buffer, buf);
    }
}


// helper function
static void print_pop_status(VaccineMonitor monitor, char *value, bool by_age) {
    // The parameter value = [country] virusName date1 date2
    int cols=-1;
    char ** values = parse_line(value, &cols, FIELD_SEPARATOR);
    char * date1 = cols <= 2 ? "1-1-0" : values[cols - 2];
    char *date2 = cols <= 2 ? "30-12-9999" : values[cols - 1];
    char * virusName = cols <= 2 ? values[cols - 1] : values[cols - 3];
    // for pretty conditioning
    bool dates_valid = check_date(date1) && check_date(date2) && dates_cmp(date1, date2) <= 0;
    bool there_is_country = (cols == 4 || cols == 2);
    
    // dates check
    if (!dates_valid)
        error_flag = true;

    // check if the virus exists in the database, if not fail.
    VirusInfo dummy_v = virus_info_create(virusName, BF_HASH_FUNC_COUNT+1, 1, 0.5);
    Pointer key = NULL;
    if (!error_flag && !(key = list_node_get_entry(monitor->virus_info, list_find(monitor->virus_info, dummy_v))))
        error_flag = true;
    
    // get it to a type'd mode, for manipulation ease (avoid constant casting)
    VirusInfo v = (VirusInfo)key;

    if (!error_flag) {
        // for shorter naming
        List l = monitor->citizen_lists_per_country;
        u_int32_t age_cnt[] = {0, 0, 0, 0};
        u_int32_t vaccd_by_age[] = {0, 0, 0, 0};
        // determine if there is a country
        if (there_is_country) {
            // if there is a country then we must pass all the people in it
            Pointer dummy_c = country_index_create(values[0]);
            CountryIndex c = (CountryIndex)list_node_get_entry(l, list_find(l, dummy_c));
            if (c != NULL)
                population_status_in_country(c, v->vaccinated, v->not_vaccinated, date1, date2, vaccd_by_age, age_cnt);
            else
                error_flag = true;

            country_index_destroy(dummy_c);

            // put the answer to the ans_buffer
            if (!error_flag)
                print_status(c->country, by_age, vaccd_by_age, age_cnt);
        } else {
            // there is no country so we have to pass them all 
            ListNode cid_node = list_get_head(l);
            while (cid_node) {
                CountryIndex c = (CountryIndex)list_node_get_entry(l, cid_node);

                // get the vaccinated count
                memset(age_cnt, 0, sizeof(u_int32_t)*4);
                memset(vaccd_by_age, 0, sizeof(u_int32_t) * 4);
                population_status_in_country(c, v->vaccinated, v->not_vaccinated, date1, date2, vaccd_by_age, age_cnt);
                cid_node = list_get_next(l, cid_node);

                // put the answer to the ans_buffer (concat it)
                print_status(c->country, by_age, vaccd_by_age, age_cnt);   
            }
        }
    }
    // free the allocated memory to avoid leaks
    virus_info_destroy(dummy_v);
    for (int i = 0; i < cols; i++) free(values[i]);
    free(values);
    if (error_flag)
        sprintf(error_msg, "ERROR");
}

static void population_status(VaccineMonitor monitor, char *value) {
    print_pop_status(monitor, value, false);
}

static void pop_status_by_age(VaccineMonitor monitor, char *value) {
    print_pop_status(monitor, value, true);
}

static void vaccinate_now(VaccineMonitor monitor, char *value) {
    // we have to reformat the given values to an existing record
    char buf[BUFSIZ];
    time_t now;
    time(&now);
    struct tm *tm = localtime(&now);
    sprintf(buf, "%s YES %02d-%02d-%02d", value, tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900);
    // now call the respective routine
    insert_record(monitor, buf, true);
}

void visit_vacc_rec(Pointer vr) {
    VaccRec vacc_rec = (VaccRec)vr;
    Person p = vacc_rec->p;
    printf("%s %s %s %s %d\n", p->citizenID, p->firstName, p->lastName, p->country_t->country, p->age);
}

static void list_not_vaccinated_persons(VaccineMonitor monitor, char *value) {
    // value = virusName
    VirusInfo dummy_v = virus_info_create(value, BF_HASH_FUNC_COUNT+1, 1, 0.5);
    VirusInfo v = list_node_get_entry(monitor->virus_info, list_find(monitor->virus_info, dummy_v));
    if (!v) {
        error_flag = true;
        sprintf(error_msg, "ERROR");
    }

    if (!error_flag) {
        sl_apply(v->not_vaccinated, visit_vacc_rec);
    }
    virus_info_destroy(dummy_v);
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
    m->citizen_lists_per_country = list_create(country_index_cmp, country_index_destroy);
    m->virus_info = list_create(virus_info_cmp, virus_info_destroy);

    if (input_filename) {
        // insert all the valid records of citizens from this file
        printf("Inserting from file '%s' ...\n", input_filename);
        insert_from_file(m, input_filename);
        #ifdef DEBUG
        ht_print_keys(m->citizens, visit);
        #endif
    }

    return m;
}

void vaccine_monitor_destroy(VaccineMonitor m) {
    list_destroy(&(m->citizen_lists_per_country));
    ht_destroy(m->citizens);
    list_destroy(&(m->virus_info));
    free(m);
}

bool vaccine_monitor_act(VaccineMonitor monitor, int expr_index, char *value) {
    switch (expr_index) {
    case 0: // command = vaccineStatusBloom , value = "citizenID virusName"
            vaccine_status_bloom(monitor, value);
            if (error_flag)
                print_error(false);
            else 
                printf("%s\n", ans_buffer);
            
            memset(ans_buffer, 0, BUFSIZ);
        break;

    case 1: // command = vaccineStatus, values = citizenID [virusName]
            vaccine_status(monitor, value);
            if (error_flag)
                print_error(false);
            else
                printf("%s\n", ans_buffer);
            memset(ans_buffer, 0, BUFSIZ);
        break;

    case 2: // command = populationStatus, values = [country] virusName date1 date2
            population_status(monitor, value);
            if (error_flag)
                print_error(false);
            else 
                printf("%s\n", ans_buffer);
            memset(ans_buffer, 0, BUFSIZ);
        break;

    case 3: // command = popStatusByAge, values = values = [country] virusName date1 date2
            pop_status_by_age(monitor, value);
            if (error_flag)
                print_error(false);
            else 
                printf("%s\n", ans_buffer);
            memset(ans_buffer, 0, BUFSIZ);
        break;

    case 4: // command = insertCitizenRecord, values = citizenID firstName lastName country age virusName YES/NO [date]
            insert_record(monitor, value, true);
            if (error_flag)
                print_error(false);
            else
                printf("%s\n", ans_buffer);
            memset(ans_buffer, 0, BUFSIZ);
            break;

    case 5:  // command = vaccinateNow, values = citizenID firstName lastName country age virusName
            
            vaccinate_now(monitor, value);
            if (error_flag)
                print_error(false);
            else
                printf("%s\n", ans_buffer);
            memset(ans_buffer, 0, BUFSIZ);
            break;

    case 6:  // command = list-nonVaccinated-Persons, values = virusName
            list_not_vaccinated_persons(monitor, value); // TO-DO
            if (error_flag)
                print_error(false);
            else
                printf("%s\n", ans_buffer);
            memset(ans_buffer, 0, BUFSIZ);
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