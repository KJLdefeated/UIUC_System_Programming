/**
 * critical_concurrency
 * CS 341 - Fall 2024
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
    queue *q = malloc(sizeof(queue));
    q->size = 0;
    q->max_size = max_size;
    q->head = NULL;
    q->tail = NULL;
    pthread_cond_init(&q->cv, NULL);
    pthread_mutex_init(&q->m, NULL);
    return q;
}

void queue_destroy(queue *this) {
    /* Your code here */
    pthread_mutex_destroy(&this->m);
    pthread_cond_destroy(&this->cv);
    queue_node *current = this->head;
    while(current != NULL) {
        queue_node *temp = current;
        current = current->next;
        free(temp);
    }
    free(this);
}

void queue_push(queue *this, void *data) {
    /* Your code here */
    pthread_mutex_lock(&this->m);
    while(this->max_size > 0 && this->size >= this->max_size) {
        pthread_cond_wait(&this->cv, &this->m);
    }
    this->size++;
    queue_node *new_node = malloc(sizeof(queue_node));
    new_node->data = data;
    new_node->next = NULL;
    if(this->head == NULL) {
        this->head = new_node;
        this->tail = new_node;
    } else {
        this->tail->next = new_node;
        this->tail = new_node;
    }
    pthread_cond_signal(&this->cv);
    pthread_mutex_unlock(&this->m);
}

void *queue_pull(queue *this) {
    /* Your code here */
    pthread_mutex_lock(&this->m);
    while(this->size == 0) {
        pthread_cond_wait(&this->cv, &this->m);
    }
    this->size--;
    queue_node *temp = this->head;
    void *data = temp->data;
    this->head = this->head->next;
    if(this->head == NULL) {
        this->tail = NULL;
    }
    free(temp);
    pthread_cond_signal(&this->cv);
    pthread_mutex_unlock(&this->m);
    return data;
}
