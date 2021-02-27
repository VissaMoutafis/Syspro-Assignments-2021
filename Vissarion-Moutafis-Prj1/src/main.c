/*
**  Implemented by Vissarion Moutafis
*/
#include <stdio.h>
#include "Types.h"
#include "Utilities.h"

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
    }

}