#pragma once

#include <stdio.h>

#include "BF.h"
#include "HT.h"
#include "List.h"
#include "SL.h"
#include "Types.h"
#include "Utilities.h"
#include "TravelMonitor.h"
#include "Monitor.h"

// Person manipulation methods

// create a person from a data line in the file
Person str_to_person(char *record);

// create a person instance
void *create_person(char * citizenID, char *firstName, char *lastName, CountryIndex country_t, int age, char *virusName, char *vaccinated, char *date, bool deep_copy);

// destroy a person instance
void person_destroy(void *_p);

// compare 2 strings that interpret very large numbers
int compare_numeric_str(char *s1, char *s2);

// function to compare 2 people
int person_cmp(void *p1, void *p2);

// function to hash a person's citizen ID
u_int32_t person_hash(void *p);

// function to completely de-allocate a person's memory
void person_complete_destroy(void *p);

// Function to check the format of the person
bool check_person_constistency(char *attrs[], int cols);

// Function to check if 2 people are the same.
bool person_equal(Person p1, Person p2);

// Country Index Manipulation methods

void *country_index_create(char *country);
int country_index_cmp(void *c1, void *c2);
u_int32_t country_index_hash(void *c);
void country_index_destroy(void *c);


// Virus Info Manipulation methods

void *virus_info_create(char *virusName, int bloom_size, int sl_height, float sl_factor);
int virus_info_cmp(void *v1, void *v2);
u_int32_t virus_info_hash(void *v);
void virus_info_destroy(void *_v);


// Vaccination Records Manipulation

void *vacc_rec_create(Person p, char *date, bool deep);
void vacc_rec_destroy(Pointer _vr);
int vacc_rec_cmp(Pointer v1, Pointer v2);


// Travel Monitor Extension: structs and manipulators

// RequestRec manipulators
Pointer request_record_create(char *date, char *countryTo);
int request_record_compare(Pointer _r1, Pointer _r2);
void request_record_destroy(Pointer _r);

// BFTuple manipulators
Pointer bftuple_create(char *country, BF bf);
int bftuple_compare(Pointer _bft1, Pointer _bft2);
void bftuple_destroy(Pointer _bft);

// VirusStats manipulators
Pointer virus_stats_create(char *virus_name);
int virus_stats_compare(Pointer _vs1, Pointer _vs2);
u_int32_t virus_stats_hash(Pointer _vs);
void virus_stats_destroy(Pointer _vs);
// extra stuff
void virus_stats_add_bf(VirusStats vs, char *country, BF bf);
void virus_stats_add_rec(VirusStats vs, RequestRec rec, bool accepted);