/**
 * deadlock_demolition
 * CS 341 - Fall 2024
 */
#include "libdrm.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Global variables for testing
drm_t *drm1, *drm2;
pthread_t thread1, thread2;

void test_lock_unlock(){
    drm1 = drm_init();
    thread1 = pthread_self();
    assert(drm_wait(drm1, &thread1) == 1);
    assert(drm_post(drm1, &thread1) == 1);
    drm_destroy(drm1);
}

void test_double_lock(){
    drm1 = drm_init();
    assert(drm_wait(drm1, &thread1) == 1);
    assert(drm_wait(drm1, &thread1) == 0);
    drm_destroy(drm1);
}

void test_unlock_without_lock(){
    drm1 = drm_init();
    assert(drm_post(drm1, &thread1) == 0);
    drm_destroy(drm1);
}

void* deadlock_thread_function(void* arg){
    int drm_num = *((int*)arg);
    pthread_t thread_id = pthread_self();

    if (drm_num == 1) {
        drm_wait(drm1, &thread_id);
        sleep(1);
        drm_wait(drm2, &thread_id);
        drm_post(drm1, &thread_id);
        drm_post(drm2, &thread_id);
    } else {
        drm_wait(drm2, &thread_id);
        sleep(1);
        drm_wait(drm1, &thread_id);
        drm_post(drm2, &thread_id);
        drm_post(drm1, &thread_id);
    }

    return NULL;
}

void test_deadlock(){
    drm1 = drm_init();
    drm2 = drm_init();

    pthread_t thread1, thread2;
    int drm1_num = 1;
    int drm2_num = 2;

    pthread_create(&thread1, NULL, deadlock_thread_function, &drm1_num);
    pthread_create(&thread2, NULL, deadlock_thread_function, &drm2_num);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    drm_destroy(drm1);
    drm_destroy(drm2);
}

int main() {
    // TODO your tests here
    // test_lock_unlock();
    // test_double_lock();
    // test_unlock_without_lock();
    test_deadlock();
    printf("All tests passed!\n");
    return 0;
}
