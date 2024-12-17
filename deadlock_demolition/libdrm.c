/**
 * deadlock_demolition
 * CS 341 - Fall 2024
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

struct drm_t {
    int locked;
    pthread_t owner;
};

graph *resource_allocation_graph = NULL;

bool graph_has_cycle_util(graph *g, void *vertex, set *visited, set *rec_stack) {
    set_add(visited, vertex);
    set_add(rec_stack, vertex);
    vector *adjacent_vertices = graph_neighbors(g, vertex);
    for (size_t i = 0; i < vector_size(adjacent_vertices); i++) {
        void *adjacent_vertex = vector_get(adjacent_vertices, i);
        if (set_contains(rec_stack, adjacent_vertex)) {
            return true;
        }
        if (set_contains(visited, adjacent_vertex) == false) {
            if (graph_has_cycle_util(g, adjacent_vertex, visited, rec_stack)) {
                return true;
            }
        }
    }
    adjacent_vertices = graph_antineighbors(g, vertex);
    for (size_t i = 0; i < vector_size(adjacent_vertices); i++) {
        void *adjacent_vertex = vector_get(adjacent_vertices, i);
        if (set_contains(rec_stack, adjacent_vertex)) {
            return true;
        }
        if (set_contains(visited, adjacent_vertex) == false) {
            if (graph_has_cycle_util(g, adjacent_vertex, visited, rec_stack)) {
                return true;
            }
        }
    }
    set_remove(rec_stack, vertex);
    return false;
}

bool graph_has_cycle(graph *g) {
    set *visited = shallow_set_create();
    set *rec_stack = shallow_set_create();
    vector *vertices = graph_vertices(g);
    for (size_t i = 0; i < vector_size(vertices); i++) {
        void *vertex = vector_get(vertices, i);
        if (set_contains(visited, vertex) == false) {
            if (graph_has_cycle_util(g, vertex, visited, rec_stack)) {
                return true;
            }
        }
    }
    return false;
}

bool graph_has_edge(graph *g, void *s, void *t) {
    vector *neighbors = graph_neighbors(g, s);
    for (size_t i = 0; i < vector_size(neighbors); i++) {
        if (vector_get(neighbors, i) == t) {
            return true;
        }
    }
    return false;
}

drm_t *drm_init() {
    /* Your code here */
    if(resource_allocation_graph == NULL) {
        resource_allocation_graph = shallow_graph_create();
    }
    struct drm_t *drm = malloc(sizeof(drm_t));
    drm->locked = 0;
    drm->owner = 0;
    return drm;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    if (resource_allocation_graph == NULL || drm == NULL || thread_id == NULL) {
        return 0;
    }
    if (graph_contains_vertex(resource_allocation_graph, drm) == false) {
        return 0;
    }
    if (graph_contains_vertex(resource_allocation_graph, thread_id) == false) {
        return 0;
    }
    if (graph_has_edge(resource_allocation_graph, thread_id, drm)) {
        graph_remove_edge(resource_allocation_graph, thread_id, drm);
        drm->locked = 0;
        return 1;
    }
    return 0;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    if (resource_allocation_graph == NULL || drm == NULL || thread_id == NULL) {
        return 0;
    }
    if (graph_contains_vertex(resource_allocation_graph, drm) == false) {
        graph_add_vertex(resource_allocation_graph, drm);
    }
    if (graph_contains_vertex(resource_allocation_graph, thread_id) == false) {
        graph_add_vertex(resource_allocation_graph, thread_id);
    }

    // Check if the drm already owns the lock
    if (graph_has_edge(resource_allocation_graph, thread_id, drm)) {
        return 0;
    }

    if (drm->locked){
        graph_add_edge(resource_allocation_graph, thread_id, drm);
        if (graph_has_cycle(resource_allocation_graph)) {
            graph_remove_edge(resource_allocation_graph, thread_id, drm);
            #ifdef DEBUG
            write(1, "Deadlock detected\n", 19);
            #endif
            return 0;
        }
        while (drm->locked) {
            // Wait for the drm to be unlocked
        }
    }

    drm->locked = 1;
    drm->owner = *thread_id;
    graph_add_edge(resource_allocation_graph, thread_id, drm);
    return 1;
}

void drm_destroy(drm_t *drm) {
    /* Your code here */
    if (drm){
        if (resource_allocation_graph && graph_contains_vertex(resource_allocation_graph, drm)) {
            graph_remove_vertex(resource_allocation_graph, drm);
        }
        free(drm);
    }
}