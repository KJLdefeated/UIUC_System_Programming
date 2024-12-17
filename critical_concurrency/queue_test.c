/**
 * critical_concurrency
 * CS 341 - Fall 2024
 */
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"

void check_create(){
    queue *q = queue_create(10);
    assert(q != NULL);
    assert(q->size == 0);
    assert(q->max_size == 10);
    assert(q->head == NULL);
    assert(q->tail == NULL);
    queue_destroy(q);
}

void check_push_pull(){
    queue *q = queue_create(10);
    int data = 5;
    queue_push(q, &data);
    assert(q->size == 1);
    assert(q->head->data == &data);
    assert(q->tail->data == &data);
    int *pulled = queue_pull(q);
    assert(*pulled == 5);
    assert(q->size == 0);
    assert(q->head == NULL);
    assert(q->tail == NULL);
    queue_destroy(q);
}

typedef struct {
    struct queue *q;
    int data;
} push_args;

void* help_queue_push(push_args *args){
    queue_push(args->q, &args->data);
    return NULL;
}

void check_threads(){
    struct queue *q = queue_create(10);
    pthread_t t1, t2;
    int data1 = 5;
    int data2 = 10;
    push_args args1 = {q, data1};
    push_args args2 = {q, data2};
    pthread_create(&t1, NULL, (void*)help_queue_push, &args1);
    pthread_create(&t2, NULL, (void*)help_queue_push, &args2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    assert(q->size == 2);
    int *pulled1 = queue_pull(q);
    int *pulled2 = queue_pull(q);
    assert(*pulled1 == 5);
    assert(*pulled2 == 10);
    assert(q->size == 0);
    queue_destroy(q);
}

int main(int argc, char **argv) {
    check_create();
    check_push_pull();
    check_threads();
    printf("All tests passed!\n");
    return 0;
}
