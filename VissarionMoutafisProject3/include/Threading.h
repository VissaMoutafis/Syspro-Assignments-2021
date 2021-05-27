#pragma once

#include <pthread.h>

#include "CB.h"
#include "Types.h"
#include "Utilities.h"

#define my_perror(s, e) fprintf(stderr, "%s:%s\n", s, strerror(e))

typedef struct thread_args {
    void **resources;               // array of void pointers to resources
    pthread_mutex_t **mutex;        // array of mutexes pointers that control resources
    pthread_cond_t **condition;     // array of conditions if needed
    int num_threads;                // the number of total threads
    int thread_id;                  // array that contains a numeric id that makes easier the
                                    // identification (from 0 to num_threads-1)
    pthread_cond_t *turn;           // pointer to the condition that determines the turn of a mutex

} ThreadArgs;


// Create 'num_threads' threads put their ids in the 'threads' array passing them 'args' as arguments
// during the __thread_routine call
void create_n_threads(int num_threads, pthread_t threads[], void *(*__thread_routine)(void *), ThreadArgs *args);


// join 'num_threads' threads with each other
void join_threads(int num_threads, pthread_t threads[]);