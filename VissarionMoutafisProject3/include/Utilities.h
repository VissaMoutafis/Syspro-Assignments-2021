/**
*	Syspro Project 3
*	 Written By Vissarion Moutafis sdi1800119
**/
 
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

// check if a date is in the proper format and is has valid values
bool check_date(char *date);

// check if a date is in range [date1, date2]
bool check_date_in_range(char *date, char *min_date, char *max_date);

// compare dates return a positive number if date1>date2, 0 if equal, a negative number if date1<date2
int dates_cmp(char *date1, char *date2);

// clean any garbage in a stream
void clean_stream(FILE **_stream);

// argument parsing/checking utilities

// print error along with a usage message
void print_arg_error(char *usage);

// <argc>, <argv> are the classic variables passed by main.
// <values> is the static array that the corresponding argument values are
// returned (malloc'd) <allowed_args> is the array of allowed argument flags
// return true in success, otherwise return false
bool parse_args(int argc, char *argv[], char *values[], char *allowed_args[], int num_allowed_args);