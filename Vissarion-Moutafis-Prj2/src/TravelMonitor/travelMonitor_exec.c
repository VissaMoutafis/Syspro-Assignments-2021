#include "TravelMonitor.h"

#include "TTY.h"

static char *usage = "Usage: \n    ~$ ./travelMonitor –m numMonitors -b bufferSize -s sizeOfBloom -i input_dir";

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
    if (argc != 9) {
        error_i = 1;
        return false;
    }

    // parse the arguments
    for (int i = 1; i < argc; i+=2) {
        int index=-1;
        if (check_arg(argv[i], allowed_args, (argc-1)/2, &index)) {
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

    // make sure the first three args are numeric
    if (!is_numeric(values[0]) || !is_numeric(values[1]) || !is_numeric(values[2])) {
        error_i = 4;
        return false;
    }

    return true;
}

int main(int argc, char ** argv) {
    char *values[4] = {NULL, NULL, NULL, NULL};
    char *allowed_args[] = {"-m", "-b", "-s", "-i"};
    if (!check_arguments(argc, argv, values, allowed_args)) {
        fprintf(stderr, "Error in arguments (%s)\n%s\n", error_string[error_i], usage);
        exit(1);
    }
    // if the checks are ok then go set the appropriate variables
    int numMonitors = DEF_NUM_MONITORS;
    if (atoi(values[0]) > 0)
        numMonitors = atoi(values[0]);
    else
        fprintf(stderr, "The numMonitors arg is non-positive. Falling back to default: %d", DEF_NUM_MONITORS);

    u_int32_t bufferSize = DEF_BUFFER_SIZE;
    if (atoi(values[1]) > 0)
        bufferSize = atoi(values[1]);
    else
        fprintf(stderr, "The bufferSize arg is non-positive. Falling back to default: %u", DEF_BUFFER_SIZE);

    size_t sizeOfBloom = DEF_BLOOM_SIZE;
    if (atoi(values[2]) > 0)
        sizeOfBloom = atoi(values[2]);
    else
        fprintf(stderr, "The sizeOfBloom arg is non-positive. Falling back to default: %u", DEF_BLOOM_SIZE);

    // Create a monitor
    TravelMonitor m = travel_monitor_create("testdir", 40, 1, 7);

    // WRITE MAIN FLOW CODE BELOW

    // WRITE MAIN FLOW CODE ABOVE

    travel_monitor_destroy(m);
}