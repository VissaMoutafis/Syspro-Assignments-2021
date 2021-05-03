#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/wait.h>
#include <signal.h>
#include <poll.h>

typedef void *Pointer; //We will use the Pointer notation for Item and/or Key type

typedef int (*Compare)(Pointer item1, Pointer item2); //Function to compare 2 items given by the user

typedef void (*ItemDestructor)(Pointer item); //Function to delete the items since the user allocates the memory for the items all alone

typedef void (*Apply)(Pointer item); //Function to help the user define the way the items are manipulated

typedef Apply Visit;                // Func to print the items (just for ease of usage)

typedef u_int32_t (*Hash_Func)(Pointer entry); //Function to hash the entry to a specific integer

// hash function needed (Given by instructors)
unsigned long hash_i(unsigned char *str, unsigned int i);

//  entry for the hash tables that contain the bloom filters
typedef struct virus_info_tuple * VirusInfo;

typedef struct list_per_country * CountryIndex;

typedef struct person{
    char *citizenID;
    char *firstName;
    char *lastName;
    CountryIndex country_t;
    int age;
    char *virusName;
    bool vaccinated;
    char *date;
} *Person;




