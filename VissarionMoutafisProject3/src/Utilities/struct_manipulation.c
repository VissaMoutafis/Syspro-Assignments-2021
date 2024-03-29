/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
/*
** Basic Struct manipulation functions
** Implemented by Vissarion Moutafis sdi1800119
*/

#include "StructManipulation.h"

// VaccRec Manipulation

void *vacc_rec_create(Person p, char *date, bool deep) {
    VaccRec vr = calloc(1, sizeof(*vr));
    vr->p = p;
    if (deep && date) {
        vr->date = calloc(strlen(date)+1, sizeof(char));
        strcpy(vr->date, date);
    }

    return (void *)vr;
}

void vacc_rec_destroy(Pointer _vr) {
    VaccRec vr = (VaccRec)_vr;
    if(vr->date)free(vr->date);
    free(vr);
}

int vacc_rec_cmp(Pointer v1, Pointer v2) {
    Person p1, p2;
    p1 = ((VaccRec)v1)->p;
    p2 = ((VaccRec)v2)->p;

    return person_cmp(p1, p2);
}

// Virus Info manipulation

// function to create a brand new virus info struct and return it casted in void*
void *virus_info_create(char *virusName, int bloom_size, int sl_height, float sl_factor) {
    VirusInfo v = calloc(1, sizeof(*v));

    v->virusName = calloc(strlen(virusName)+1, sizeof(char));
    strcpy(v->virusName, virusName);
    v->bf = bf_create(BF_HASH_FUNC_COUNT, bloom_size);
    v->vaccinated = sl_create(vacc_rec_cmp, vacc_rec_destroy, sl_height, sl_factor);
    v->not_vaccinated = sl_create(vacc_rec_cmp, vacc_rec_destroy, sl_height, sl_factor);

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


// Person Manipulation

void *create_person(char * citizenID, char *firstName, char *lastName, struct list_per_country *country_t, int age, char *virusName, char *vaccinated, char *date, bool deep_copy) {
    // the vaccinated answer is either yes or no
    if (!( !strcmp(vaccinated, "YES") || !strcmp(vaccinated, "NO") ))
        return NULL;

    Person p = calloc(1, sizeof(*p));

    p->age = age;
    p->vaccinated = (strcmp(vaccinated, "YES") == 0);
    p->country_t = country_t;

    if (deep_copy) {
        p->citizenID = calloc(strlen(citizenID)+1, sizeof(char));
        strcpy(p->citizenID, citizenID);

        p->firstName = calloc(strlen(firstName) + 1, sizeof(char));
        strcpy(p->firstName, firstName);

        p->lastName = calloc(strlen(lastName) + 1, sizeof(char));
        strcpy(p->lastName, lastName);

        p->virusName = calloc(strlen(virusName) + 1, sizeof(char));
        strcpy(p->virusName, virusName);

        if (date) {
            p->date = calloc(strlen(date) + 1, sizeof(char));
            strcpy(p->date, date);
        } else 
            p->date = NULL;

    } else {
        p->citizenID = citizenID;
        p->firstName = firstName;
        p->lastName = lastName;
        p->virusName = virusName;
        p->date = date;
    }

    return (void*)p;
}

void person_destroy(void *_p) {
    Person p = (Person)_p;

    // country_t and virusName are redudant elements so we will let the user free them accordingly

    free(p->citizenID);
    free(p->firstName);
    free(p->lastName);
    free(p->date);
    free(p->virusName);
    free(p);
}

int compare_numeric_str(char *s1, char *s2) {
    // the idea is that we get 2 strings that cannot be accomodated by the casual system types,
    // so we have to compare them as strings
    char *p1, *p2;
    p1 = s1;
    p2 = s2;
    // first we have to remove the padding of zeros, if it exists for each string.
    // i.e. s1 = "0001" -> "1"
    while (p1[0] && p1[0] == '0' && p1[1]) p1++;
    while (p2[0] && p2[0] == '0' && p2[1]) p2++;

    // now if one string is larger than the other then its larger as a number
    // if the strings has equal lengths then we should return the value of strlen
    int l1 = strlen(p1), l2 = strlen(p2);
    if (l1 > l2)        return 1;
    else if (l1 < l2)   return -1;
    else                return strcmp(p1, p2); 
}

int person_cmp(void *p1, void *p2) {
    return compare_numeric_str(((Person)p1)->citizenID, ((Person)p2)->citizenID);
}

u_int32_t person_hash(void *p) {
    char * id = ((Person)p)->citizenID;
    while(id[0] && id[0] == '0' && id[1]) id++;
    return hash_i((unsigned char*)id, strlen(id));
}

void person_complete_destroy(void *_p) {
    Person p = (Person)_p;

    country_index_destroy(p->country_t);
    person_destroy(p);
}

// Function to check the format of the person
bool check_person_constistency(char *attrs[], int cols) {
    return cols <= 8 && cols >= 7                                   &&
           attrs[0] && atoi(attrs[0]) <= 9999                       &&
           attrs[1]                                                 &&
           attrs[2]                                                 &&
           attrs[3]                                                 &&
           is_numeric(attrs[4]) && atoi(attrs[4]) > 0 && atoi(attrs[4])<=120&&
           atoi(attrs[4]) > 0                                       &&
           attrs[5]                                                 &&
           attrs[6]                                                 &&
           (!strcmp(attrs[6], "YES") || !strcmp(attrs[6], "NO"))    && // vaccinated =yes/no
           ( (!strcmp(attrs[6], "YES") && cols == 8 && check_date(attrs[7]))    || 
           (!strcmp(attrs[6], "NO") && cols == 7));    // either vaccinated or no date
}

// Function to check if 2 people are the same.
bool person_equal(Person p1, Person p2) {
    return !compare_numeric_str(p1->citizenID, p2->citizenID)
        && !strcmp(p1->firstName, p2->firstName)
        && !strcmp(p1->lastName, p2->lastName)
        && !strcmp(p1->country_t->country, p2->country_t->country)
        && p1->age == p2->age;
}

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


// Extension for Travel Monitor structs

Pointer request_record_create(char *date, char * countryTo){
    RequestRec r = calloc(1, sizeof(*r));
    r->date = calloc(strlen(date)+1, sizeof(char));
    strcpy(r->date, date);
    r->countryTo = calloc(strlen(countryTo)+1, sizeof(char));
    strcpy(r->countryTo, countryTo);

    return (Pointer)r;
}

int request_record_compare(Pointer _r1, Pointer _r2) {
    RequestRec r1, r2;
    r1 = (RequestRec)_r1;
    r2 = (RequestRec)_r2;
    return strcmp(r1->date, r2->date);
}

void request_record_destroy(Pointer _r) {
    free(((RequestRec)_r)->date);
    free(((RequestRec)_r)->countryTo);
    free(_r);
}

Pointer bftuple_create(char *country, BF bf) {
    assert(bf);
    assert(country);

    BFTuple bft = calloc(1, sizeof(*bft));
    bft->country = calloc(strlen(country)+1, sizeof(char));
    strcpy(bft->country, country);
    bft->bf = bf;

    return (Pointer)bft;
}

int bftuple_compare(Pointer _bft1, Pointer _bft2) {
    BFTuple bft1, bft2;
    bft1 = (BFTuple)_bft1;
    bft2 = (BFTuple)_bft2;
    return strcmp(bft1->country, bft2->country);
}

void bftuple_destroy(Pointer _bft) {
    assert(_bft);

    BFTuple bft = (BFTuple)_bft;
    free(bft->country);
    free(bft);
}

int dummy_compare(Pointer a, Pointer b) {
    return 1;
}
void bf_destroy_wrapper(Pointer bf) {
    bf_destroy((BF)bf);
}
Pointer virus_stats_create(char *virus_name) {
    assert(virus_name);

    VirusStats vs = calloc(1, sizeof(*vs));
    vs->virus_name = calloc(strlen(virus_name)+1, sizeof(char));
    strcpy(vs->virus_name, virus_name);
    vs->accepted = list_create(request_record_compare, request_record_destroy);
    vs->rejected = list_create(request_record_compare, request_record_destroy);
    vs->bf_per_countries = sl_create(bftuple_compare, bftuple_destroy, COUNTRY_SL_HEIGHT, 0.5);
    vs->unique_bfs = list_create(dummy_compare, bf_destroy_wrapper);
    return (Pointer)vs;
}

int virus_stats_compare(Pointer _vs1, Pointer _vs2) {
    VirusStats vs1, vs2;
    vs1 = (VirusStats)_vs1;
    vs2 = (VirusStats)_vs2;
    return strcmp(vs1->virus_name, vs2->virus_name);
}

u_int32_t virus_stats_hash(Pointer _vs) {
    VirusStats vs = (VirusStats)_vs;
    return hash_i((unsigned char *)vs->virus_name, strlen(vs->virus_name));
}

void virus_stats_destroy(Pointer _vs) {
    VirusStats vs = (VirusStats)_vs;
    free(vs->virus_name);
    sl_destroy(vs->bf_per_countries);
    list_destroy(&(vs->accepted));
    list_destroy(&(vs->rejected));
    list_destroy(&(vs->unique_bfs));
    free(vs);
}

void virus_stats_add_bf(VirusStats vs, char *country, BF bf) {
    Pointer old = NULL;
    sl_insert(vs->bf_per_countries, bftuple_create(country, bf), false, &old);


    #ifdef DEBUG
    assert(old == NULL);
    struct bf_tuple bft = {.country = country};
    sl_search(vs->bf_per_countries, &bft);
    #endif
}

void virus_stats_add_rec(VirusStats vs, RequestRec rec, bool accepted) {
    List l = accepted ? vs->accepted : vs->rejected;
    list_insert(l, rec, true);
}