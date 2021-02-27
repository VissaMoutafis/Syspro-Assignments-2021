#pragma once

#include "Types.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

// create a string from the line that is currently pointed by *_stream
char *make_str(FILE **_stream_);

// get the number of the lines of filename 
size_t fget_lines(char *filename);

// 
char **parse_line(char *data_str, int *columns, char* sep);


bool is_numeric(char* str);
