#include "Threading.h"

// mutexes for resources
pthread_mutex_t mtx_cb = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mtx_monitor = PTHREAD_MUTEX_INITIALIZER;  // mutex that locks when we apply a change into
                              // the Monitor struct
pthread_mutex_t mtx_check_end = PTHREAD_MUTEX_INITIALIZER;  // mutex that locks everytime we check for end of execution

// conditionals and their mutexes
pthread_cond_t cond_cb_full = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx_cb_full = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_cb_empty = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mtx_cb_empty = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mtx_turn = PTHREAD_MUTEX_INITIALIZER;

// Thread Utility Wrappers //

void create_n_threads(int num_threads, pthread_t threads[], void *(*__thread_routine)(void *), ThreadArgs *args) {
    // basic assertions
    assert(threads);
    assert(num_threads > 0);

    // error num
    int err;
    // loop to call <num_threads> threads to execute <__thread_routine>, with
    // arguments <args>
    for (int i = 0; i < num_threads; i++) {
        if ((err = pthread_create(&threads[i], NULL, __thread_routine, args))) {
            my_perror("Problem creating thread", err);
            exit(1);
        }
    }
}

void join_threads(int num_threads, pthread_t threads[]) {
    int err;
    for (int i = 0; i < num_threads; i++) {
        printf("(%d) Waiting for (%lu)\n", getpid(), threads[i]);
        if ((err = pthread_join(threads[i], NULL))) {
            my_perror("Error in joining thread", err);
            exit(1);
        }
        #ifdef DEBUG
        printf("Thread (%lu) exited\n", threads[i]);
        #endif
    }
}
