/*
**  Implemented by Vissarion Moutafis
*/
#include <stdio.h>
#include "Types.h"
#include "Utilities.h"
#include "TTY.h"
#include "VaccineMonitor.h"

int error_i = 0;
char *error_string[] = {
                        "", 
                        "Wrong Number of Arguments",
                        "False Argument", 
                        "False positioning of Arguments",
                        "False Type of Argument value"
                        };

static bool check_arg(char *arg, char *args[], int size, int *index) {
    for (int i = 0; i < size; i ++) {
        if (strcmp(args[i], arg) == 0) {
            *index = i;
            return true;
        }
    }
    return false;
}

static bool check_arguments(int argc, char *argv[], char *values[], char * allowed_args[]) {
    // check for proper number of arguments
    if (argc != 5) {
        error_i = 1;
        return false;
    }

    // parse the arguments
    for (int i = 1; i < argc; i+=2) {
        int index=-1;
        if (check_arg(argv[i], allowed_args, 2, &index)) {
            // index is the index number of the proper argument listing
            values[index] = argv[i+1];
            
            // make sure that the value is not another argument, this cannot be allowed
            if (check_arg(values[index], allowed_args, 2, &index)) {
                error_i = 3;
                return false;
            }

        } else {
            error_i = 2;
            return false;
        }
    }
    // make sure the second argument value (bloom filter size) is numeric
    if (!is_numeric(values[1])) {
        error_i = 4;
        return false;
    }

    return true;
}

int main(int argc, char * argv[]) {
    // First check arguments 
    char *values[2]={NULL, NULL};                        // the values of the arguments
    char *allowed_args[2] = {"-c", "-b"};   // the 

    if (!check_arguments(argc, argv, values, allowed_args)) {
        fprintf(stderr, "Error in arguments (%s) . \nUsage: \n    ~$ ./vaccineMonitor -c citizenRecordsFile -b bloomSize\n", error_string[error_i]);
        exit(1);
    }



    vaccine_monitor_initialize();
    VaccineMonitor monitor = vaccine_monitor_create(values[0], atoi(values[1]), 10, 0.5);

    // basic loop of the program
    while (!is_end) {
        // first get the input expression from the tty
        char *expr = get_input();
        // now parse it to the expression part and the value part
        char ** parsed_expr = parse_expression(expr); // format: /command value(s)

        // clarify the input with proper assignments
        char *command = parsed_expr[0];
        char *value = parsed_expr[1];
        int expr_index;

        // CREATE THE VACCINE MONITOR 

        // now we have to check if the expression was ok based on the array of allowed formats
        if (check_format(command, &expr_index) && check_value_list(value, expr_index)) {
            // puts("The command was ok, you can proceed to pass them to vaccineMonitor with:");
            // printf("Command (index): '%s' (%d)\n", command, expr_index);
            // printf("Value(s): '%s'\n", value);
            
            // we will try to execute the command. If vaccine monitor fails then we will print the 
            // error message to stderr
            if (!vaccine_monitor_act(monitor, expr_index, value)) {
                fprintf(stderr, "%s\n", error_msg);
            }
        } else 
            help();

        if (expr) free(expr);
        if (parsed_expr[0]) free(parsed_expr[0]);
        if (parsed_expr[1]) free(parsed_expr[1]);
        if (parsed_expr) free(parsed_expr);
    }

    // de-allocate the memory for every struct you have.
    // DE-ALLOCATE THE MEMORY OF THE VACCINE MONITOR
    vaccine_monitor_destroy(monitor);
    
    return 0;
}