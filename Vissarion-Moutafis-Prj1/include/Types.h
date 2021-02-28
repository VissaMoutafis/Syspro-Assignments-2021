#pragma once

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdbool.h>
#include <assert.h>

typedef void *Pointer; //We will use the Pointer notation for Item and/or Key type

typedef int (*Compare)(Pointer item1, Pointer item2); //Function to compare 2 items given by the user

typedef void (*ItemDestructor)(Pointer item); //Function to delete the items since the user allocates the memory for the items all alone

typedef void (*Visit)(Pointer item); //Function to help the user define the way the items are printed

typedef u_int32_t (*Hash_Func)(Pointer entry); //Function to hash the entry to a specific integer

// hash function needed
unsigned long hash_i(unsigned char *str, unsigned int i);

typedef struct {
    char *citizenID;
    char *firstName;
    char *lastName;
    char *country;
    int age;
    char *virusName;
    char *date;
} Person;