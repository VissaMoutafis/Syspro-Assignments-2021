#include "Threading.h"

#include "CB.h"

// Thread Utility Wrappers //

void create_n_threads(int num_threads, pthread_t threads[],
                      void *(*__thread_routine)(void *), ThreadArgs *args) {
    // basic assertions
    assert(threads);
    assert(num_threads > 0);

    // error num
    int err;
    // loop to call <num_threads> threads to execute <__thread_routine>, with
    // arguments <args>
    for (int i = 0; i < num_threads; i++) {
        ((ThreadArgs *)args)->thread_id=i;
        if (err = pthread_create(&threads[i], NULL, __thread_routine, args)) {
            my_perror("Problem creating thread", err);
            exit(1);
        }
    }
}

void join_threads(int num_threads, pthread_t threads[]) {
    int err, status;
    for (int i = 0; i < num_threads; i++) {
        if (err = pthread_join(threads[i], NULL)) {
            my_perror("Error in joining thread", err);
            exit(1);
        }
#ifdef DEBUG
        printf("Thread (%lu) exited with status %d\n", threads[i], status);
#endif
    }
}

// Test cases
#define NUM_THREADS 3

void *print_str(void *args) {
    CB cb = (CB)(((ThreadArgs *)args)->resources[0]);
    pthread_mutex_t *cb_m, *end_m, *mtx_empty, *mtx_full, *turn_mtx;
    cb_m = ((ThreadArgs *)args)->mutex[0];
    end_m = ((ThreadArgs *)args)->mutex[1];
    turn_mtx = ((ThreadArgs *)args)->mutex[2];
    mtx_empty = ((ThreadArgs *)args)->mutex[3];
    mtx_full = ((ThreadArgs *)args)->mutex[4];

    bool *end = (bool *)(((ThreadArgs *)args)->resources[1]);
    int thread_id = pthread_self();
    int num_threads = ((ThreadArgs *)args)->num_threads;
    pthread_cond_t *cb_full, *cb_empty, *turn;
    cb_full = ((ThreadArgs *)args)->condition[0];
    cb_empty = ((ThreadArgs *)args)->condition[1];
    turn = ((ThreadArgs *)args)->turn;

    printf("%d starting\n",thread_id);

    while (1) {
        bool is_end = false;
        pthread_mutex_lock(turn_mtx);
        // check if it is my turn
        pthread_cond_wait(turn, turn_mtx);
        pthread_mutex_unlock(turn_mtx);

        printf("(%d)got turn\n", thread_id);

        pthread_mutex_lock(end_m);
        printf("(%d) end = %s\n", thread_id, *end ? "true" : "false");
        is_end = *end;
        pthread_mutex_unlock(end_m);
        if (is_end) break;

        pthread_mutex_lock(cb_m);  // lock the mutex for atomicity
        printf("(%d) got cb\n", thread_id);
        
        pthread_mutex_lock(mtx_empty);
        while (cb_is_empty(cb)){
        pthread_cond_wait(cb_empty, mtx_empty);  // check for empty buffer
        }
        printf("(%d) got cb_empty\n", thread_id);
        char *s = cb_get(cb);
        
        pthread_mutex_unlock(mtx_empty);
        
        pthread_mutex_lock(mtx_full);
        pthread_cond_signal(cb_full);  // signal that the buffer is not full now
        pthread_mutex_unlock(mtx_full);

        pthread_mutex_unlock(cb_m);  // release the mutex


        printf("(%d) got %s\n", thread_id, s);
    }

    printf("(%d) exited\n", thread_id);

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    ThreadArgs d;

    // args
    CB cb = cb_create(5, NULL);
    bool end = false;
    void *args[] = {cb, &end};

    // mutexes
    pthread_mutex_t cb_m = PTHREAD_MUTEX_INITIALIZER,
                    end_m = PTHREAD_MUTEX_INITIALIZER,
                    turn_m = PTHREAD_MUTEX_INITIALIZER,
                    mtx_empty = PTHREAD_MUTEX_INITIALIZER,
                    mtx_full = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t *ms[5] = {&cb_m, &end_m, &turn_m, &mtx_empty, &mtx_full};

    // conditions
    pthread_cond_t cb_full, cb_empty, turn;
    pthread_cond_init(&cb_full, 0);
    pthread_cond_init(&cb_empty, 0);
    pthread_cond_init(&turn, 0);
    pthread_cond_t *cs[2] = {&cb_full, &cb_empty};

    d.resources = args;
    d.mutex = ms;
    d.condition = cs;
    d.turn = &turn;
    d.num_threads = NUM_THREADS;

    for (int i = 1; i < argc; i++) cb_add(cb, argv[i]);

    int err, status;
    pthread_t threads[3];
    memset(threads, 0, 3 * sizeof(pthread_t));

    create_n_threads(NUM_THREADS, threads, print_str, &d);    

    char *user_input = NULL;
    do {
        user_input = make_str(&stdin);
        puts("read done");

        pthread_mutex_lock(&turn_m);
        pthread_cond_signal(&turn);
        pthread_mutex_unlock(&turn_m);

        pthread_mutex_lock(&cb_m);
        pthread_mutex_lock(&mtx_full);
        while (cb_is_full(cb)) {
            pthread_cond_wait(&cb_full, &mtx_full);  // check if full
            puts("check full done");
        }
        cb_add(cb, user_input);
        pthread_mutex_unlock(&mtx_full);
        pthread_mutex_unlock(&cb_m);
        puts("add done");
        
        pthread_mutex_lock(&mtx_empty);
        pthread_cond_signal(&cb_empty);
        pthread_mutex_unlock(&mtx_empty);
        puts("signal empty done");
    } while (strncmp(user_input, "END", 3) != 0);

    puts("Exit");
    pthread_mutex_lock(&end_m);
    end = true;
    pthread_mutex_unlock(&end_m);
    puts("Set end");
    for (int i = 0; i < NUM_THREADS; i++){
        pthread_cond_signal(&turn);  // make sure no one blocks there
        pthread_mutex_unlock(&end_m);
    }
    puts("signaled empty");

    join_threads(NUM_THREADS, threads);
    puts("joined all threads");

    cb_destroy(cb);
    pthread_mutex_destroy(&cb_m);
    pthread_mutex_destroy(&end_m);
    pthread_cond_destroy(&cb_empty);
    pthread_cond_destroy(&cb_full);
}