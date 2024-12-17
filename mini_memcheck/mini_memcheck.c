/**
 * mini_memcheck
 * CS 341 - Fall 2024
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

meta_data *head = NULL;

size_t total_memory_requested = 0;

size_t total_memory_freed = 0;

size_t invalid_addresses = 0;

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here
    if (request_size == 0) {
        return NULL;
    }
    total_memory_requested += request_size;
    meta_data* tmp = (meta_data*)malloc(sizeof(meta_data) + request_size);
    if (head == NULL) {
        head = tmp;
        head->request_size = request_size;
        head->filename = filename;
        head->instruction = instruction;
        head->next = NULL;
        return (void*)(head + 1);
    }
    meta_data* current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = tmp;
    current->next->request_size = request_size;
    current->next->filename = filename;
    current->next->instruction = instruction;
    current->next->next = NULL;
    return (void*)(current->next + 1);
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    if (num_elements == 0 || element_size == 0) {
        return NULL;
    }
    total_memory_requested += num_elements * element_size;
    meta_data* tmp = (meta_data*)calloc(1, sizeof(meta_data) + num_elements * element_size);
    if (head == NULL) {
        head = tmp;
        head->request_size = num_elements * element_size;
        head->filename = filename;
        head->instruction = instruction;
        head->next = NULL;
        return (void*)(head + 1);
    }
    meta_data* current = head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = tmp;
    current->next->request_size = num_elements * element_size;
    current->next->filename = filename;
    current->next->instruction = instruction;
    current->next->next = NULL;
    return (void*)(current->next + 1);
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    if (payload == NULL) {
        return mini_malloc(request_size, filename, instruction);
    }
    if (request_size == 0) {
        mini_free(payload);
        return NULL;
    }
    meta_data* current = head;
    while (current != NULL) {
        if (current + 1 == payload) {
            break;
        }
        current = current->next;
    }
    if (current == NULL) {
        invalid_addresses++;
        return NULL;
    }
    if (current->request_size == request_size) {
        return payload;
    }
    meta_data* newPayload = (meta_data*)realloc(current, sizeof(meta_data) + request_size);
    if (current->request_size > request_size) {
        total_memory_freed += current->request_size - request_size;
    } else {
        total_memory_requested += request_size - current->request_size;
    }
    newPayload->request_size = request_size;
    // Write out the total_memory_requested and total_memory_freed
    // fprintf(stderr, "Total memory requested: %zu\n", total_memory_requested);
    // fprintf(stderr, "Total memory freed: %zu\n", total_memory_freed);
    return (void*)(newPayload + 1);
}

void mini_free(void *payload) {
    // your code here
    if (payload == NULL) {
        return;
    }
    // fprintf(stderr, "Total memory requested: %zu\n", total_memory_requested);
    // fprintf(stderr, "Total memory freed: %zu\n", total_memory_freed);
    // fprintf(stderr, "Invalid addresses: %zu\n", invalid_addresses);
    meta_data* current = head;
    meta_data* prev = NULL;
    while (current != NULL) {
        if (current + 1 == payload) {
            break;
        }
        prev = current;
        current = current->next;
    }
    if (current == NULL) {
        invalid_addresses++;
        return;
    }
    if (prev == NULL) {
        head = current->next;
    } else {
        prev->next = current->next;
    }
    total_memory_freed += current->request_size;
    free(current);
}
