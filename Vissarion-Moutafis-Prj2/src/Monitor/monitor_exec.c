#include "TTY.h"
#include "Types.h"
#include "Utilities.h"
#include "Monitor.h"
#include "IPC.h"

GetResponse get_response = monitor_get_response;

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
            // make sure no arg is
            // numeric
            if (is_numeric(argv[i+1])) {
                error_i = 4;
                return false;
            }
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

    return true;
}

// call as: ./monitor -i inputFifo -o outputFifo
int main(int argc, char * argv[]) {
    // First check arguments 
    char *values[2]={NULL, NULL};                        // the values of the arguments
    char *allowed_args[2] = {"-i", "-o"};               // argument flags (input -i, output -o) 

    if (!check_arguments(argc, argv, values, allowed_args)) {
        fprintf(stderr, "Error in arguments (%s) \nUsage: \n    ~$ ./monitor -i inputFifo -o outputFifo\n", error_string[error_i]);
        exit(1);
    }
    
    char **dirs = NULL;
    int dir_num = 0;
    void *ret_args[] = {&dirs, &dir_num};
    
    int in_fd = open(values[0], O_RDONLY | O_NONBLOCK);
    int out_fd = open(values[1], O_WRONLY);
    get_response(NULL, get_dirs, 0, in_fd, ret_args);
    
    printf("dirnum: %d\n", dir_num);
    for (int i = 0; i < dir_num; i++) puts(dirs[i]);
    // FM fm = fm_create(dirs, dir_num);

    // monitor_initialize();
    // Monitor monitor = monitor_create(fm, 1000, 10, 0.5);
    exit(1);
}