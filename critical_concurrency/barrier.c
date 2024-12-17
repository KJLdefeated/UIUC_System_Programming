/**
 * critical_concurrency
 * CS 341 - Fall 2024
 */
#include "barrier.h"

// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    if (barrier->times_used != 0) return -1;
    if (pthread_mutex_destroy(&barrier->mtx) != 0) return -1;
    if (pthread_cond_destroy(&barrier->cv) != 0) return -1;
    return 0;
}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    if (num_threads == 0 || barrier == NULL) return -1;
    barrier->n_threads = num_threads;
    barrier->count = 0;
    barrier->times_used = 0;
    if (pthread_mutex_init(&barrier->mtx, NULL) != 0) return -1;
    if (pthread_cond_init(&barrier->cv, NULL) != 0) return -1;
    return 0;
}

int barrier_wait(barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mtx);
    barrier->count++;
    unsigned int times_used = barrier->times_used;
    if (barrier->count == barrier->n_threads) {
        barrier->times_used++;
        barrier->count = 0;
        pthread_cond_broadcast(&barrier->cv);
    } else {
        while (times_used == barrier->times_used) {
            pthread_cond_wait(&barrier->cv, &barrier->mtx);
        }
    }
    pthread_mutex_unlock(&barrier->mtx);
    return 0;
}
