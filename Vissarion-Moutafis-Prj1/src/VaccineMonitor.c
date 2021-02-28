#include <stdio.h>
#include "HT.h"
#include "SL.h"
#include "BF.h"
#include "List.h"
#include "Utilities.h"

struct vaccine_monitor {
    // General indexing for citizens
    HT citizens;
    HT citizen_lists_per_country;
    // Skip lists according to vaccination details per citizen. 
    // One per virus. 
    // We keep them in HT for fast indexing
    // {key:virusName, item:skip list for (not)vaccinated people} 
    HT vaccinated_people_lists;
    HT not_vaccinated_people_lists;
    // Bloom filters for quick check whether a person is not vaccinated for a virus
    // {key: virusName, item: skip list for (not)vaccinated people}
    HT bf_per_virus;
};

//  entry for the hash tables that contain the bloom filters
typedef struct bf_tuple {
    char *virusName;
    BF bf;
} *BFEntry;

// entry for the hash tables that contain the skip lists for vaccinated or not, people
typedef struct vaccinated_list_tuple {
    char *virusName;
    SL sl;
} *SLEntry;

// entry for the hash table that contain the lists of people that belong in a country
typedef struct lists_per_country {
    char *country;
    List l;
} *CountryEntry;

