/**
*	Syspro Project 2
*	 Written By Vissarion Moutafis sdi1800119
**/
 
#include <stdio.h>
#include <string.h>

#include "Utilities.h"
#include "Types.h"

// ###################### Caller can change this to apply to his constraints
// ############### // all the possible commands for the tty API of the app
int pos_cmds_len;

// for format checking
char *allowed_formats[5];

// for help printing
char *possible_commands[5];

//properties of the value_lists for different commands
int max_values[5];
int min_values[5];


// Methods to print messages
void help(void);

// get the input from the cmd
char *get_input(void);

// check if the format of the command is appropriate. return the index 
// of the allowed formats array based on the expr given
bool check_format(char *expr, int *expr_index);

// perfom a swallow check in the value list (given as string)
bool check_value_list(char *value_list, int expr_index);

// parse the expression and return an array with command and value (both strings)
char **parse_expression(char *expr);