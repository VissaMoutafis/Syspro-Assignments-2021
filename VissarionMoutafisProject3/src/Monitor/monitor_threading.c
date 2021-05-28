#include "Monitor.h"
#include "Threading.h"

void insert_records(Monitor monitor, char *file_path) {
    int len = 0;
    char **recs = NULL;
    fm_read_from_file(monitor->fm, file_path, &recs, &len);

    for (int i = 0; i < len; i++) {
        pthread_mutex_lock(&mtx_monitor);
        insert_record(monitor, recs[i], update_at_insert);
        pthread_mutex_unlock(&mtx_monitor);
        free(recs[i]);
    }
    free(recs);
}

static bool is_full(CB cb) {
    pthread_mutex_lock(&mtx_cb);
    bool ret = cb_is_full(cb);
    pthread_mutex_unlock(&mtx_cb);
    return ret;
}

static bool is_empty(CB cb) {
    pthread_mutex_lock(&mtx_cb);
    bool ret = cb_is_empty(cb);
    pthread_mutex_unlock(&mtx_cb);
    return ret;
}

static bool thread_is_end(void) {
    // check if we done
    pthread_mutex_lock(&mtx_check_end);
    bool __is_end = thread_end;
    pthread_mutex_unlock(&mtx_check_end);
    return __is_end;
}

void *monitor_thread_routine(void *_args) {
    // args->resources = {monitor struct}
    Monitor monitor = (Monitor)_args;

    // basic algorithm
    // 1. Wait for your turn.
    // 2. Check if the programm exited => break the loop.
    // 3. Acquire the circular buffer mutex.
    // 4. Check if the empty condition-var is signaled (wait if needed) and get
    // one file path, then unlock the cb mutex.
    // 5. Signal the cb_full condition so that the main thread knows that
    // circular buffer is not full anymore.
    // 6. Lock the monitor mutex and perfom the insertion. Then unlock the
    // mutex.
    // 7. At exit free any necessary memory.

    while (!thread_is_end()) {
        // first we wait our turn
        pthread_mutex_lock(&mtx_turn);
        // check if we done
        if (thread_is_end()) break;
        
        pthread_mutex_lock(&mtx_cb_empty);
        // check if the circular buffer is not empty
        while (is_empty(monitor->cb) && !thread_is_end())
            pthread_cond_wait(&cond_cb_empty, &mtx_cb_empty);
        pthread_mutex_unlock(&mtx_cb_empty);

        // check if we done
        if (!thread_is_end()) {
            // if not do stuff
            // if we get the OK, get one file path ATOMICALLY
            pthread_mutex_lock(&mtx_cb);  // acquire the mutex
            char *file_path = NULL;
            char *_file_path = cb_get(monitor->cb);
            if (_file_path) {
                puts(_file_path);
                file_path = calloc(strlen(_file_path) + 1, sizeof(char));
                strcpy(file_path, _file_path);
                if (strcmp(_file_path, "END")) free(_file_path);
            }
            // unlock the cb_empty mutex
            pthread_mutex_unlock(&mtx_cb);  // unlock the mutex

            // signal the cb_full condition-var
            pthread_mutex_lock(&mtx_cb_full);
            pthread_cond_signal(&cond_cb_full);
            pthread_mutex_unlock(&mtx_cb_full);

            // insert the records into the monitor
            if (file_path) {
                if (strcmp(file_path, "END"))
                    insert_records(monitor, file_path);
                free(file_path);
            }
        }

        pthread_mutex_unlock(&mtx_turn);
    }
    pthread_mutex_lock(&mtx_check_end);
    threads_done++;
    pthread_mutex_unlock(&mtx_check_end);
    printf("%d-%lu EXIT\n", getpid(), pthread_self());

    pthread_exit(NULL);
}

void monitor_producer_routine(Monitor monitor, char **files, int flen) {
    for (int i = 0; i < flen; i++) {
        // first wake up a thread
        pthread_mutex_lock(&mtx_cb_full);
        // try to insert something (check if cb full if not add it ATOMICALY)
        while (is_full(monitor->cb))
            pthread_cond_wait(&cond_cb_full, &mtx_cb_full);
        pthread_mutex_unlock(&mtx_cb_full);

        pthread_mutex_lock(&mtx_cb);
        // the cb is not full add another record atomically
        cb_add(monitor->cb, files[i]);
        pthread_mutex_unlock(&mtx_cb);

        // wake a consumer up
        pthread_mutex_unlock(&mtx_turn);

        // signal cb_empty condition-var since there is at least one record
        pthread_mutex_lock(&mtx_cb_empty);
        pthread_cond_signal(&cond_cb_empty);
        pthread_mutex_unlock(&mtx_cb_empty);
    }
}

static bool check_all_threads_done() {
    pthread_mutex_lock(&mtx_check_end);
    bool ret = (threads_done == num_threads);
    pthread_mutex_unlock(&mtx_check_end);

    return ret;
}

void clean_up_threads(Monitor monitor) {
    // join threads
    pthread_mutex_lock(&mtx_check_end);
    thread_end = true;
    pthread_mutex_unlock(&mtx_check_end);
    threads_done = 0;
    while (!check_all_threads_done()) {
        // unlock any threads that are blocked on the turn mutex
        pthread_mutex_unlock(&mtx_turn);

        // signal all the mutexes waiting on cb_empty 
        pthread_cond_broadcast(&cond_cb_empty);
    }
    join_threads(num_threads, threads);
    free(threads);
    // free mutexes and conditions
    pthread_mutex_destroy(&mtx_cb);
    pthread_mutex_destroy(&mtx_cb_empty);
    pthread_mutex_destroy(&mtx_cb_full);
    pthread_mutex_destroy(&mtx_check_end);
    pthread_mutex_destroy(&mtx_monitor);
    pthread_mutex_destroy(&mtx_turn);

    pthread_cond_destroy(&cond_cb_full);
    pthread_cond_destroy(&cond_cb_empty);
}