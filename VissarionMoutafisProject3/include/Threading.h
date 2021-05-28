#pragma once

#include <pthread.h>

#include "CB.h"
#include "Types.h"
#include "Utilities.h"

#define my_perror(s, e) fprintf(stderr, "%s:%s\n", s, strerror(e))

typedef void *ThreadArgs;


// Create 'num_threads' threads put their ids in the 'threads' array passing them 'args' as arguments
// during the __thread_routine call
void create_n_threads(int num_threads, pthread_t threads[], void *(*__thread_routine)(void *), ThreadArgs *args);


// join 'num_threads' threads with each other
void join_threads(int num_threads, pthread_t threads[]);