#include <stdio.h>
#include "HT.h"
#include "SL.h"
#include "BF.h"
#include "List.h"
#include "Utilities.h"
#include "VaccineMonitor.h"

char *error_table[] = {"ERROR: RECORD OMMITED"};

bool error_flag = false;
char *error_msg = NULL;

static void print_error(bool exit_fail) {
    fprintf(stderr, "%s\n", error_msg);
    error_flag = false;
    error_msg = NULL;
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
    HT virus_info;
    int bloom_size;
    int sl_height;
    float sl_factor;
};

//  entry for the hash tables that contain the bloom filters
typedef struct virus_info_tuple {
    char *virusName;                // key of the tuple
    BF bf;                          // bloom filter to check if a person is not vaccinated
    SL vaccinated;                  // skip list with vaccinated people
    SL not_vaccinated;              // skip list with non-vaccinated people
} *VirusInfo;

typedef struct list_per_country {
    char *country;
    HT citizen_ht;
} *CountryIndex;



// Virus Info manipulation
// function to create a brand new virus info struct and return it casted in void*
void *virus_info_create(char *virusName, int bloom_size, int sl_height, float sl_factor) {
    VirusInfo v = calloc(1, sizeof(*v));

    v->virusName = calloc(strlen(virusName)+1, sizeof(char));
    strcpy(v->virusName, virusName);
    v->bf = bf_create(BF_HASH_FUNC_COUNT, bloom_size);
    v->vaccinated = sl_create(person_cmp, NULL, sl_height, sl_factor);
    v->not_vaccinated = sl_create(person_cmp, NULL, sl_height, sl_factor);

    return (void*)v; 
}

// for virus info comparison
int virus_info_cmp(void *v1, void *v2) {
    return strcmp(((VirusInfo)v1)->virusName, ((VirusInfo)v2)->virusName);
}

// for virus info hashing, when inderted in a hash table
u_int32_t virus_info_hash(void *v) {
    return hash_i((unsigned char *)((VirusInfo)v)->virusName, (unsigned int)strlen(((VirusInfo)v)->virusName));
}

// to destroy the virus info 
void virus_info_destroy(void *_v) {
    VirusInfo v = (VirusInfo)_v;
    free(v->virusName);
    bf_destroy(v->bf);
    sl_destroy(v->vaccinated);
    sl_destroy(v->not_vaccinated);
    free(v);
}

// Function to insert a record in the structs of the according virusInfo tuple
// If update flag is true then we will update the record if it exists
// In the right spirit this is also a vaccination operation for the citizens
static void virus_info_insert(VaccineMonitor monitor, Person p, bool update) {
    // First we will check if the virus info is created
    VirusInfo dummy = virus_info_create(p->virusName, monitor->bloom_size, monitor->sl_height, monitor->sl_factor);
    Pointer key;

    if (ht_contains(monitor->virus_info, dummy, &key)) {
        Pointer person = NULL, person_dummy = NULL;
        VirusInfo v = (VirusInfo)key;

        // pick the appropriate skip list to search and/or insert
        SL sl = (p->vaccinated == true) ? v->vaccinated : v->not_vaccinated;
        
        // try to insert to the list 
        if (!(person = sl_search(sl, p))) {
            sl_insert(sl, p, false, &person_dummy);
            bf_insert(v->bf, p);
            #ifdef DEBUG
            assert(bf_contains(v->bf, p));
            assert(sl_search(sl, p));
            #endif
        } else if (update && sl == v->not_vaccinated && p->vaccinated) {
            // the update flag is true
            // and the person is in the not-vaccinated group and he got vaccinated, since
            // there is no point to process people already vaccinated and/or update based on
            // records that refer to not-vaccinated people 

            // delete him from the non-vaccinated group
            if (!sl_delete(v->not_vaccinated, person, false, &person_dummy)) {
                fprintf(stderr, "\nPROBLEM DURING DELETION OF %s.\n", ((Person)person)->citizenID);
                exit(1);
            }

            // debug
            #ifdef DEBUG
            assert(!sl_search(v->not_vaccinated, person));
            assert(!sl_search(v->not_vaccinated, p));
            assert(person == person_dummy);
            #endif

            // insert the person instance into the vaccinated group
            sl_insert(v->vaccinated, person, false, &person);
            
            // debug
            #ifdef DEBUG
            assert(sl_search(v->vaccinated, p));
            #endif

            person_destroy(p);
        }
        virus_info_destroy(dummy);
    } else {
        // there is no instance of the virusInfo, so we will insert the dummy rec 
        // and we will also insert the person in the bloom filter and 
        // into the appropriate skip list
        ht_insert(monitor->virus_info, dummy, false, &key);
        // insert into BF
        bf_insert(dummy->bf, p);
        //insert into the appropriate skip list
        SL sl = (p->vaccinated == true) ? dummy->vaccinated : dummy->not_vaccinated;
        sl_insert(sl, p, false, &key);

        #ifdef DEBUG
        assert(ht_contains(monitor->virus_info, dummy, &key));
        assert(sl_search(sl, p));
        #endif
    }
}
// Country Index manipulation

// function to create a brand new country index struct.
void *country_index_create(char *country) {
    CountryIndex c = calloc(1, sizeof(*c));

    c->country = calloc(strlen(country)+1, sizeof(char));
    strcpy(c->country, country);
    c->citizen_ht = ht_create(person_cmp, person_hash, NULL);

    return (void *)c;
}

// for country index comparison 
int country_index_cmp(void *c1, void *c2) {
    return strcmp(((CountryIndex)c1)->country, ((CountryIndex)c2)->country);
}

// function to hash the country index entry struct for appropriate handling in the respective hash table (index)
u_int32_t country_index_hash(void *c) {
    return hash_i((unsigned char *)((CountryIndex)c)->country, (unsigned int)strlen(((CountryIndex)c)->country));
}

// to free the memory beign kept by the country index structs
void country_index_destroy(void *c) {
    free(((CountryIndex)c)->country);
    ht_destroy(((CountryIndex)c)->citizen_ht);
    free(c);
}

// Function to insert an entry to  the country index of a vaccine monitor
static void country_index_insert(VaccineMonitor monitor, Person p) {
    CountryIndex dummy = country_index_create(p->country);
    Pointer key;
    // check if we already have a list
    if (ht_contains(monitor->citizen_lists_per_country, dummy, &key)) {
        // there is already an index list so,
        // we just add the record into it, if it does not already exists
        CountryIndex c = (CountryIndex)key;
        Pointer old;
        if (!ht_contains(c->citizen_ht, p, &old)) 
            ht_insert(c->citizen_ht, p, false, &old);
        // destroy the dummy node
        country_index_destroy(dummy);

        // for debuging purposes
        #ifdef DEBUG
        assert(ht_contains(c->citizen_ht, p, &old));
        #endif
    } else {
        // there is no country index in the hash. So we must insert the dummy
        // one there and then add the record to the index list as a first entry
        // (head)
        Pointer old;
        ht_insert(monitor->citizen_lists_per_country, dummy, false, &key);
        ht_insert(dummy->citizen_ht, p, false, &old);
        #ifdef DEBUG
        assert(ht_contains(monitor->citizen_lists_per_country, dummy, &key));
        assert(ht_contains(dummy->citizen_ht, p, &old));
        #endif
    }
}

// Vaccine Monitor Utilities

// Function to check the format of the person
static bool check_person_constistency(char *attrs[], int cols) {
    return cols <= 8 && cols >= 7                                   &&
           attrs[0]                                                 &&
           attrs[1]                                                 &&
           attrs[2]                                                 &&
           attrs[3]                                                 &&
           is_numeric(attrs[4])                                     &&
           atoi(attrs[4]) > 0                                       &&
           attrs[5]                                                 &&
           attrs[6]                                                 &&
           (!strcmp(attrs[6], "YES") || !strcmp(attrs[6], "NO"))    && // vaccinated =yes/no
           ( (!strcmp(attrs[6], "YES") && cols == 8) || cols == 7);    // either vaccinated or no date
}

// Function to check if 2 people are the same.
static bool person_equal(Person p1, Person p2) {
    return !strcmp(p1->citizenID, p2->citizenID)
        && !strcmp(p1->firstName, p2->firstName)
        && !strcmp(p1->lastName, p2->lastName)
        && !strcmp(p1->country, p2->country)
        && p1->age == p2->age;
}

// create a person from a data line in the file
static Person str_to_person(char *record) {
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
    Person p = create_person(parsed_rec[0], parsed_rec[1], parsed_rec[2],
                            parsed_rec[3], atoi(parsed_rec[4]), parsed_rec[5],
                            parsed_rec[6], cols == 8? parsed_rec[7] : NULL, true);
    
    // free the memory of the parse array
    for (int i = 0; i < cols; i++) free(parsed_rec[i]);
    free(parsed_rec);

    return p;
}

// Insert the record into the monitor. 
// If the update flag is one then, if we find an instance of it, about a specific virus
// either ommit the incosistent record, or 
// actually update the proper virus info lists and bloom filter 
void insert_record(VaccineMonitor monitor, char *record, bool update) {
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
            error_msg = error_table[0];
            error_flag = true;
        } else {
            // key = p if it does not exists in the general citizen hash,
            // other wise key = already existing key
            virus_info_insert(monitor, key, update);
        }

        // if the person exists there is no reason to keep the instance we created
        if (exists)
            person_destroy(p);
         
    } else {
        error_msg = error_table[0];
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

// Basic Vaccine Monitor Methods

void vaccine_monitor_initialize(void) { 
    is_end = false; 
    error_msg = NULL;
}

void vaccine_monitor_finalize(void) {
    is_end = true;
}

VaccineMonitor vaccine_monitor_create(char *input_filename, int bloom_size, int sl_height, float sl_factor) {
    VaccineMonitor m = calloc(1, sizeof(*m));

    m->bloom_size = bloom_size;
    m->sl_height = sl_height;
    m->sl_factor = sl_factor;
    m->citizens = ht_create(person_cmp, person_hash, person_destroy);
    m->citizen_lists_per_country = ht_create(country_index_cmp, country_index_hash, country_index_destroy);
    m->virus_info = ht_create(virus_info_cmp, virus_info_hash, virus_info_destroy);

    if (input_filename) {
        // insert all the valid records of citizens from this file
        // insert from file
        
        // implement the function to insert a citizen

        // implement the function to vaccinate a citizen
        printf("Inserting from file '%s' ...\n", input_filename);
        if (!insert_from_file(m, input_filename)) {
            vaccine_monitor_destroy(m);
            fprintf(stderr, "ERROR: PROBLEM DURING INSERTION FROM FILES.\n");
            exit(1);
        }
    }

    return m;
}

void vaccine_monitor_destroy(VaccineMonitor m) {
    ht_destroy(m->citizen_lists_per_country);
    ht_destroy(m->citizens);
    ht_destroy(m->virus_info);
    free(m);
}

bool vaccine_monitor_act(VaccineMonitor monitor, int expr_index, char *value) {
    switch (expr_index)
    {
    case 0:
        
        break;

    case 1:

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