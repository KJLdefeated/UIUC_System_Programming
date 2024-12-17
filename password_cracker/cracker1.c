/**
 * password_cracker
 * CS 341 - Fall 2024
 */
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "includes/queue.h"
#include <crypt.h>
#include <string.h>
#include <stdio.h>

struct data {
    char *username;
    char *hash;
    char *password;
};

struct cracker_args {
    int thread_id;
    int num_successful;
    int num_failed;
    queue *q;
};

char* crack(char* hash, char* password, int* hash_count) {
    char* tmp = password + getPrefixLength(password);
    setStringPosition(tmp, 0);
    while(true){
        (*hash_count)++;
        struct crypt_data cdata;
        cdata.initialized = 0;
        char* guess_hash = crypt_r(password, "xx", &cdata);
        if(strcmp(hash, guess_hash) == 0){
            return password;
        }
        if(incrementString(tmp) == 0){
            return NULL;
        }
    }
}

void* cracker_helper(void* arg) {
    struct cracker_args *args = (struct cracker_args*)arg;
    queue *q = args->q;
    args->num_successful = 0;
    args->num_failed = 0;
    while(true){
        struct data *d = queue_pull(q);
        if(d == NULL){
            break;
        }
        char* username = d->username;
        char* hash = d->hash;
        char* password = d->password;
        v1_print_thread_start(args->thread_id, username);
        double start_time = getThreadCPUTime();
        int hash_count = 0;
        char* result = crack(hash, password, &hash_count);
        double end_time = getThreadCPUTime();
        if(result == NULL){
            v1_print_thread_result(args->thread_id, username, result, hash_count, end_time - start_time, 1);
            args->num_failed++;
        } else {
            v1_print_thread_result(args->thread_id, username, result, hash_count, end_time - start_time, 0);
            args->num_successful++;
        }
        free(d->username);
        free(d->hash);
        free(d->password);
        free(d);
    }
    return NULL;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    char *username = malloc(256);
    char *hash = malloc(256);
    char *password = malloc(256);
    queue *q = queue_create(-1);
    while(scanf("%s %s %s", username, hash, password) != EOF){
        struct data *d = malloc(sizeof(struct data));
        d->username = strdup(username);
        d->hash = strdup(hash);
        d->password = strdup(password);
        queue_push(q, d);
    }
    for(size_t i=0;i<thread_count;i++) queue_push(q, NULL); // To stop the threads
    pthread_t threads[thread_count];
    struct cracker_args args[thread_count];
    for(size_t i=0;i<thread_count;i++){
        args[i].q = q;
        args[i].thread_id = i+1;
        pthread_create(&threads[i], NULL, cracker_helper, &args[i]);
    }
    for(size_t i=0;i<thread_count;i++){
        pthread_join(threads[i], NULL);
    }
    int num_successful = 0, num_failed = 0;
    for(size_t i=0;i<thread_count;i++){
        num_successful += args[i].num_successful;
        num_failed += args[i].num_failed;
    }
    v1_print_summary(num_successful, num_failed);
    queue_destroy(q);
    free(username);
    free(hash);
    free(password);
    return 0;
}
