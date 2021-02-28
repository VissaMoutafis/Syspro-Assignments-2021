/*
**  TTY-like API methods
**  Written by Vissarion Moutafis sdi1800119
*/
#include "TTY.h"


// all the possible commands for the tty API of the app
int pos_cmds_len = 8;

// for format checking
char *allowed_formats[] = {
    "vaccineStatusBloom",
    "vaccineStatus",
    "populationStatus",
    "popStatusByAge",
    "insertCitizenRecord",
    "vaccinateNow",
    "list-nonVaccinated-People",
    "exit"};

// for help printing
char *possible_commands[] = {
    "/vaccineStatusBloom citizenID virusName",
    "/vaccineStatus citizenID [virusName]",
    "/populationStatus [country] virusName date1 date2",
    "/popStatusByAge [country] virusName date1 date2",
    "/insertCitizenRecord citizenID firstName lastName jcountry age virusName YES/NO [date]",
    "/vaccinateNow citizenID firstName lastName contry age virusName",
    "/list-nonVaccinated-People virusName",
    "/exit"
};

// for value list argument checking (swallow checks)
int max_values [] = {
    2,
    2,
    4,
    4,
    8,
    6,
    1,
    0
};
int min_values [] = {
    2,
    1,
    3,
    3,
    8,
    6,
    1,
    0
};

void print_tty(void) {
    // NOTE: Maybe later add some color
    printf("\n<vaccineMonitor> /");
}

void help(void) {
    printf("Possible commands for the vaccineMonitor tty:\n");
    for (int i = 0; i < pos_cmds_len; i++) {
        printf("\t%s\n", possible_commands[i]);
    }
}

char *get_input(void) {
    char *in;

    print_tty();
    in = make_str(&stdin);
    return in;
}

bool check_format(char* expr, int *expr_index) {
    // linear search for the format
    for (int i = 0; i < pos_cmds_len; i++)
        if (strcmp(expr, allowed_formats[i]) == 0) {
            *expr_index = i;
            return true;
        }
    
    // if not found return error
    return false;
}


char **parse_expression(char * expr) {
    // we know that there could be at most 2 arguments <expression> <value string>
    // so we parse them with this mind set
    char ** parsed_expr = calloc(2, sizeof(char*)); // make an array of 2 possible arguments
    unsigned int expr_index = 0;

    for(unsigned int i = 0; expr[i]; i++) {
        if (expr[i] == ' ')
            break;
        expr_index ++;
    }
    // Now we know exactly were is the end of the command
    char * command = calloc(expr_index + 1, sizeof(char));
    strncpy(command, expr, expr_index);

    char *value = NULL ;
    if (strlen(expr) - expr_index > 0) {
        // if there is a value

        // set the value to an empty string
        value = calloc(strlen(expr) - expr_index, sizeof(char));
        // copy the letters (sorry don't know any other function to do that)
        for (unsigned int i = expr_index+1 ; expr[i]; i++)
            value[i-(expr_index+1)] = expr[i];
    }
    
    // set the command and value fields
    parsed_expr[0] = command;
    parsed_expr[1] = value;
    // we are done
    return parsed_expr;
}

// return true if value list is ok, or false of value list is broken. Part of the swallow check of the tty
bool check_value_list(char *value_list, int expr_index) {
    int values = 0;
    

    if (value_list) {
        char buf[BUFSIZ];
        memcpy(buf, value_list, strlen(value_list) + 1);
        char *val = strtok(buf, " ");
        while (val) {
            values++;
            val = strtok(NULL, " ");
        }
    }

    return values >= min_values[expr_index] && values <= max_values[expr_index];
}