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


