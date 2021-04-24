#include "VaccineMonitor.h"
#include "StructManipulation.h"

void visit(Pointer _p) {
    Person p = (Person)_p;
    printf("%s %s %s %s %d\n", p->citizenID, p->firstName, p->lastName,
           p->country_t->country, p->age);
}

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
            bf_insert(new_vi->bf, p);

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

static void travel_request(VaccineMonitor monitor, char *value) {
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
                sprintf(ans_buffer, "YES %s", vac_rec->date);
                found=true;
            }
        }
    }

    if (!found)
        sprintf(ans_buffer, "NO");

    for (int i = 0; i < values_len; i++) free(values[i]);
    free(values);
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

static void insert_from_dir(VaccineMonitor monitor, FM fm, DirectoryEntry dentry) {
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

static void insert_from_fm(VaccineMonitor monitor, FM fm) {
    // get the list of added records in the file manager
    List dir_list = fm_get_directory_list(fm);
    // iterate the directories and add their records in the monitor's structs
    for (ListNode node = list_get_head(dir_list); node != NULL; node = list_get_next(dir_list, node)) {
        DirectoryEntry dentry = list_node_get_entry(dir_list, node); 
        insert_from_dir(monitor, fm, dentry);
    }
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

VaccineMonitor vaccine_monitor_create(FM fm, int bloom_size, int sl_height, float sl_factor) {
    VaccineMonitor m = calloc(1, sizeof(*m));

    m->bloom_size = bloom_size;
    m->sl_height = sl_height;
    m->sl_factor = sl_factor;
    m->citizens = ht_create(person_cmp, person_hash, person_destroy);
    m->citizen_lists_per_country = list_create(country_index_cmp, country_index_destroy);
    m->virus_info = list_create(virus_info_cmp, virus_info_destroy);

    if (fm) {
        // insert all the valid records of citizens from the directories in the fm
        insert_from_fm(m, fm);
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
    case 0: // command: /travelRequest, value: citizenID date, countryFrom countryTo virusName
            travel_request(monitor, value);
            answer();
        break;
    case 2: // command: /addVaccinationRecords, value: country
            // add_vaccination_record(monitor, value);
            answer();
        break;

    case 3: // command: /searchVaccinationStatus, value: citizenID
        break;

    case 4: // command: /exit, value: -
            vaccine_monitor_finalize();
            break;
    default:
        return false;
        break;
    }
    return true;
}