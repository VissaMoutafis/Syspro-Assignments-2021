#pragma once

#include "Types.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// create a string from the line that is currently pointed by *_stream
char *make_str(FILE **_stream_);

// get the number of the lines of filename 
size_t fget_lines(char *filename);

// function to parse a line. Returns an array of strings with columns elements
// parse based on the sep, passed by the user
char **parse_line(char *data_str, int *columns, char* sep);

// return true if str is a numeric element
// false if does not, only, contain digits.
bool is_numeric(char* str);


// New utilities for the vaccine monitor

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
