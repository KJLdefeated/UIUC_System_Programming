/**
 * teaching_threads
 * CS 341 - Fall 2024
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct {
    int *list;
    size_t list_len;
    reducer reduce_func;
    int base_case;
    int result;
} task_t;

/* You should create a start routine for your threads. */
void* handler(void* args){
    task_t* task = (task_t*) args;
    task->result = reduce(task->list, task->list_len, task->reduce_func, task->base_case);
    return NULL;
}

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    int n_theads = (num_threads < list_len) ? num_threads : list_len;
    pthread_t threads[n_theads];
    task_t tasks[n_theads];
    int chunk = list_len / n_theads, remainder = list_len % n_theads;
    int start = 0;
    for (int i=0;i<n_theads;i++){
        tasks[i].list = list + start;
        tasks[i].list_len = chunk + (i < remainder);
        tasks[i].reduce_func = reduce_func;
        tasks[i].base_case = base_case;
        pthread_create(&threads[i], NULL, handler, &tasks[i]);
        start += tasks[i].list_len;
    }
    for (int i=0;i<n_theads;i++){
        pthread_join(threads[i], NULL);
    }
    int result = base_case;
    for (int i=0;i<n_theads;i++){
        result = reduce_func(result, tasks[i].result);
    }
    return result;
}
