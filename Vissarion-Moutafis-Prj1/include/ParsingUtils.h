#pragma once

#include "Types.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

char *make_str(FILE **_stream_);
size_t fget_lines(char *filename);
char **parse_line(char *data_str, int *columns, char* sep);
bool is_numeric(char* str);

// arg parsing

// input argc and argv from the termina
// input string array pointer to place the input, and int* for the input size
// allowed arguments THAT REQUIRE VALUE (i.e. -i "file") array for proper error checking (don't let the user check for basic errors i.e. null argument value)
void args_parser(int argc, char **argv, char ***input, int *input_size, char** allowed_arguments);
// special function to parse config file
void parse_cnfg(char* filename); 