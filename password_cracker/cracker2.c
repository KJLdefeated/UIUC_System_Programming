/**
 * password_cracker
 * CS 341 - Fall 2024
 */
#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include "includes/queue.h"
#include <crypt.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>

pthread_barrier_t barrier;

struct data {
    char *username;
    char *hash;
    char *password;
    long *start;
    long *count;
};

struct cracker_args {
    queue *q;
    pthread_mutex_t *lock;
    int *found;
    int *hash_count;
    char *password;
    int *complete;
    int thread_id;
    size_t thread_count;
};

void* crack(void* arg) {
    struct cracker_args *args = (struct cracker_args*)arg;
    queue *q = args->q;
    pthread_mutex_t *lock = args->lock;
    int thread_id = args->thread_id;
    while(true){
        struct data *d = queue_pull(q);
        if(d == NULL){
            break;
        }
        pthread_mutex_lock(lock);
        if(*args->complete == 1){
            v2_print_start_user(d->username);
            *args->complete = 0;
        }
        pthread_mutex_unlock(lock);

        double start_time = getTime(), cpu_start_time = getCPUTime();
        char* username = d->username;
        char* hash = d->hash;
        char* password = d->password;
        long *start = d->start;
        long *count = d->count;
        int hash_count = 0;
        char* tmp = password + getPrefixLength(password);
        setStringPosition(tmp, *start);
        v2_print_thread_start(thread_id, username, *start, password);
        int status = 2;
        int local_found = 0;
        while((*count) > 0){

            pthread_mutex_lock(lock);
            local_found = *args->found;
            pthread_mutex_unlock(lock);
            if(local_found == 1){
                status = 1;
                break;
            }

            hash_count++;
            
            struct crypt_data cdata;
            cdata.initialized = 0;
            char* guess_hash = crypt_r(password, "xx", &cdata);
            if(strcmp(hash, guess_hash) == 0){
                status = 0;
                
                pthread_mutex_lock(lock);
                args->password = strcpy(args->password, password);
                *args->found = 1;
                pthread_mutex_unlock(lock);

                break;
            }
            
            (*count)--;

            if(incrementString(tmp) == 0){
                status = 2;
                break;
            }
        }
        // Finish one user
        v2_print_thread_result(thread_id, hash_count, status);
        
        pthread_mutex_lock(lock);
        *args->hash_count += hash_count;
        pthread_mutex_unlock(lock);

        pthread_barrier_wait(&barrier);
        
        double end_time = getTime(), cpu_end_time = getCPUTime();
        
        pthread_mutex_lock(lock);

        if ((*args->complete) == 0) {
            v2_print_summary(username, args->password, *args->hash_count, end_time - start_time, cpu_end_time - cpu_start_time, !(*args->found));
            *args->complete = 1;
            *args->hash_count = 0;
            *args->found = 0;
        }

        pthread_mutex_unlock(lock);

        free(d->username);
        free(d->hash);
        free(d->password);
        free(d->start);
        free(d->count);
        free(d);

        pthread_barrier_wait(&barrier);
    }
    return NULL;
}

int start(size_t thread_count) {
    char username[256];
    char hash[256];
    char password[256];
    queue *q = queue_create(-1);
    pthread_mutex_t lock;
    struct cracker_args args[thread_count];
    pthread_t threads[thread_count];
    pthread_barrier_init(&barrier, NULL, thread_count);
    pthread_mutex_init(&lock, NULL);
    while(scanf("%s %s %s", username, hash, password) != EOF) {
        int unknown_letter_count = strlen(password) - getPrefixLength(password);
        for(size_t i = 0; i < thread_count; i++){
            long start_index, count;
            getSubrange(unknown_letter_count, thread_count, i + 1, &start_index, &count);
            struct data *d = malloc(sizeof(struct data));
            d->username = strdup(username);
            d->hash = strdup(hash);
            d->password = strdup(password);
            d->start = malloc(sizeof(long));
            d->count = malloc(sizeof(long));
            *(d->start) = start_index;
            *(d->count) = count;
            queue_push(q, d);
        }
    }
    for(size_t i = 0; i < thread_count; i++){
        queue_push(q, NULL);
    }
    int found = 0, complete = 1, hash_count = 0;
    char* ans_password = malloc(256);
    for(size_t i = 0; i < thread_count; i++){
        args[i].q = q;
        args[i].lock = &lock;
        args[i].found = &found;
        args[i].hash_count = &hash_count;
        args[i].password = ans_password;
        args[i].complete = &complete;
        args[i].thread_id = i + 1;
        pthread_create(&threads[i], NULL, crack, &args[i]);
    }
    for(size_t i = 0; i < thread_count; i++){
        pthread_join(threads[i], NULL);
    }
    free(ans_password);
    queue_destroy(q);
    pthread_mutex_destroy(&lock);
    pthread_barrier_destroy(&barrier);
    return 0;
}
