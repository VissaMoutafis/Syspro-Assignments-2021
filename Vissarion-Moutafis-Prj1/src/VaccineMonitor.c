#include <stdio.h>
#include "HT.h"
#include "SL.h"
#include "BF.h"
#include "List.h"
#include "Utilities.h"
#include "VaccineMonitor.h"

// all the possible commands for the tty API of the app
int pos_cmds_len = 8;

// for format checking
char *allowed_formats[] = {"vaccineStatusBloom",        "vaccineStatus",
                           "populationStatus",          "popStatusByAge",
                           "insertCitizenRecord",       "vaccinateNow",
                           "list-nonVaccinated-People", "exit"};

// for help printing
char *possible_commands[] = {
    "/vaccineStatusBloom citizenID virusName",
    "/vaccineStatus citizenID [virusName]",
    "/populationStatus [country] virusName date1 date2",
    "/popStatusByAge [country] virusName date1 date2",
    "/insertCitizenRecord citizenID firstName lastName jcountry age virusName "
    "YES/NO [date]",
    "/vaccinateNow citizenID firstName lastName contry age virusName",
    "/list-nonVaccinated-People virusName",
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
    List citizen_list;
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
    
    bf_destroy(v->bf);
    sl_destroy(v->vaccinated);
    sl_destroy(v->not_vaccinated);
    free(v);
}

// Country Index manipulation

// function to create a brand new country index struct.
void *country_index_create(char *country) {
    CountryIndex c = calloc(1, sizeof(*c));

    c->country = calloc(strlen(country)+1, sizeof(char));
    strcpy(c->country, country);
    c->citizen_list = list_create(person_cmp, NULL);

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
    list_destroy(&(((CountryIndex)c)->citizen_list));
    free(c);
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
        insert from file
        
        implement the function to insert a citizen

        implement the function to vaccinate a citizen
    }

    return m;
}

void vaccine_monitor_destroy(VaccineMonitor m) {
    ht_destroy(m->citizen_lists_per_country);
    ht_destroy(m->citizens);
    ht_destroy(m->virus_info);
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

        break;

    default:
        return 1;
        break;
    }
}