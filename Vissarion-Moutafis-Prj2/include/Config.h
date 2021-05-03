#pragma once



// for fifos 
#define FIFO_DIR "fifos"

// for logging
#define ROOT_LOG_PATH "./logs"
#define MONITOR_LOG_PATH "./logs/monitors"
#define TRAVEL_MONITOR_LOG_PATH "./logs/travel-monitor"

// defaults, travel_monitor specific
#define DEF_NUM_MONITORS 1
#define DEF_BLOOM_SIZE 100
#define DEF_BUFFER_SIZE 100

// defaults, monitor specific
#define SL_HEIGHT 10
#define SL_FACTOR 0.5


// general defaults
#define BF_HASH_FUNC_COUNT 16       // number of hash functions of a bloom filter

#define FIELD_SEPARATOR " "         // how the cmd command's value-fields are separeted

// variable to determine whether the user wants to exit the application or not
bool is_end;

char error_msg[BUFSIZ];
bool error_flag;