/**
 * parallel_make
 * CS 341 - Fall 2024
 */

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "set.h"
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <dirent.h>

pthread_mutex_t lock;

int graph_has_cycle_util(graph* g, void* vertex, set* vis, set* stack){
    set_add(vis, vertex);
    set_add(stack, vertex);
    vector* adjacent_vertices = graph_neighbors(g, vertex);
    for (size_t i = 0; i < vector_size(adjacent_vertices); i++) {
        void* adjacent_vertex = vector_get(adjacent_vertices, i);
        if (set_contains(stack, adjacent_vertex)) {
            vector_destroy(adjacent_vertices);
            return 1;
        }
        if (set_contains(vis, adjacent_vertex) == false) {
            if (graph_has_cycle_util(g, adjacent_vertex, vis, stack)) {
                vector_destroy(adjacent_vertices);
                return 1;
            }
        }
    }
    set_remove(stack, vertex);
    vector_destroy(adjacent_vertices);
    return 0;
}

void check_cycle(graph* dependency_graph) {
    set* visited = shallow_set_create();
    set* stack = shallow_set_create();
    vector* vertices = graph_neighbors(dependency_graph, "");
    for (size_t i = 0; i < vector_size(vertices); i++) {
        void* vertex = vector_get(vertices, i);
        if (set_contains(visited, vertex) == false) {
            if(graph_has_cycle_util(dependency_graph, vertex, visited, stack)){
                print_cycle_failure((char*)vertex);
                graph_remove_edge(dependency_graph, "", vertex);
            }
        }
        else if (set_contains(stack, vertex)) {
            print_cycle_failure((char*)vertex);
            graph_remove_edge(dependency_graph, "", vertex);
        }
    }
    set_destroy(visited);
    set_destroy(stack);
    vector_destroy(vertices);
}

void topological_sort_util(void* target, graph* dependency_grapth, set* visited, set* stack, vector* sorted){
    set_add(visited, target);
    set_add(stack, target);
    vector* adjacent_vertices = graph_neighbors(dependency_grapth, target);
    for (size_t i = 0; i < vector_size(adjacent_vertices); i++) {
        void* vertex = vector_get(adjacent_vertices, i);
        if (set_contains(visited, vertex) == false) {
            topological_sort_util(vertex, dependency_grapth, visited, stack, sorted);
        }
    }
    vector_push_back(sorted, target);
    vector_destroy(adjacent_vertices);
}

void topological_sort(vector* targets, graph* dependency_graph, vector* sorted) {
    set* visited = shallow_set_create();
    set* stack = shallow_set_create();
    for (size_t i = 0; i < vector_size(targets); i++) {
        void* target = vector_get(targets, i);
        if (set_contains(visited, target) == false) {
            topological_sort_util(target, dependency_graph, visited, stack, sorted);
        }
    }
    set_destroy(visited);
    set_destroy(stack);
}

struct thread_data {
    queue* q;
    graph* dependency_graph;
    vector* sorted;
};

void* thread_make(void* arg) {
    struct thread_data* data = (struct thread_data*)arg;
    queue* q = data->q;
    pthread_mutex_lock(&lock);
    graph* dependency_graph = data->dependency_graph;
    pthread_mutex_unlock(&lock);

    while (1) {
        char* target = queue_pull(q);

        if (strcmp(target, "") == 0) {
            queue_push(q, "");
            break;
        }

        // Check rule dependencies
        pthread_mutex_lock(&lock);
        vector* dependencies = graph_neighbors(dependency_graph, (void*)target);
        pthread_mutex_unlock(&lock);
        
        int flag = 0;
        int state = 1;
       
        for (size_t i = 0; i < vector_size(dependencies); i++) {
            char* dependency = vector_get(dependencies, i);
            pthread_mutex_lock(&lock);
            rule_t* rule = (rule_t*)graph_get_vertex_value(dependency_graph, dependency);
            state = rule->state;
            pthread_mutex_unlock(&lock);
            if (state == -1) {
                flag = -1;
                break;
            }
        }
        
        vector_destroy(dependencies);
        // Execute commands
        // If flag == -1, then the rule has failed
        pthread_mutex_lock(&lock);
        rule_t* rule = (rule_t*)graph_get_vertex_value(dependency_graph, target);
        state = rule->state;
        pthread_mutex_unlock(&lock);

        #ifdef DEBUG
        printf("Checking %s %d\n", target, state);
        #endif

        if (state == 1) continue;

        if (flag == 0){
            state = 1;
            for (size_t i = 0; i < vector_size(rule->commands); i++) {
                char* command = vector_get(rule->commands, i);
                if(system(command) != 0){
                    state = -1;
                    break;
                }
            }
        }
        else{
            state = -1;
        }   
        
        
        // Update rule state
        pthread_mutex_lock(&lock);
        rule->state = state;
        vector* neighbors = data->sorted;
        for (size_t i = 0; i < vector_size(neighbors); i++) {
            char* neighbor = vector_get(neighbors, i);
            rule_t* rulen = (rule_t*)graph_get_vertex_value(dependency_graph, neighbor);
            if (fopen(neighbor, "r") != NULL) {
                rulen->state = 1;
            }
            if (rulen->state) continue;
            vector* dependencies = graph_neighbors(dependency_graph, neighbor);
            int flag = 1;
            for (size_t j = 0; j < vector_size(dependencies); j++) {
                rule_t* rule = (rule_t*)graph_get_vertex_value(dependency_graph, vector_get(dependencies, j));
                if (rule->state == -1) {
                    flag = 0;
                    rulen->state = -1;
                    break;
                }
                if (rule->state != 1) {
                    flag = 0;
                    break;
                }
            }
            vector_destroy(dependencies);
            if (flag) {
                rulen->state = 2;
                queue_push(q, neighbor);
            }
        }
        vector* vertices = graph_neighbors(dependency_graph, "");
        int flag2 = 1;
        for (size_t i = 0; i < vector_size(vertices); i++) {
            rule_t* rule = (rule_t*)graph_get_vertex_value(dependency_graph, vector_get(vertices, i));
            #ifdef DEBUG
            printf("Checking %s %d\n", (char*)vector_get(vertices, i), rule->state);
            #endif
            if (rule->state == 0) {
                flag2 = 0;
                break;
            }
        }
        if (flag2) {
            queue_push(q, "");
        }
        vector_destroy(vertices);
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    // Make dependency graph
    graph* dependency_graph = parser_parse_makefile(makefile, targets);
    
    // Check for cycles
    #ifdef DEBUG
    printf("Checking for cycles\n");
    #endif
    check_cycle(dependency_graph);
    
    // Topological sort
    #ifdef DEBUG
    printf("Sorting targets\n");
    #endif
    vector *vertices = graph_neighbors(dependency_graph, "");
    #ifdef DEBUG
    for (size_t i = 0; i < vector_size(vertices); i++) {
        printf("Vertex: %s\n", (char*)vector_get(vertices, i));
    }
    #endif
    vector *sorted = shallow_vector_create();
    topological_sort(vertices, dependency_graph, sorted);

    // Insert sorted targets into job queue
    queue* job_queue = queue_create(-1);
    for(size_t i = 0; i < vector_size(sorted); i++){
        // If target already exists in path, then state is already set
        if (fopen((char*)vector_get(sorted, i), "r") != NULL) {
            rule_t* rule = (rule_t*)graph_get_vertex_value(dependency_graph, vector_get(sorted, i));
            rule->state = 1;
        }
    }
    // Insert targets with no dependencies into job queue
    for(size_t i = 0; i < vector_size(sorted); i++){
        vector* neighbors = graph_neighbors(dependency_graph, vector_get(sorted, i));
        rule_t* rulen = (rule_t*)graph_get_vertex_value(dependency_graph, vector_get(sorted, i));
        int flag = 1;
        for(size_t j = 0; j < vector_size(neighbors); j++){
            rule_t* rule = (rule_t*)graph_get_vertex_value(dependency_graph, vector_get(neighbors, j));
            if (rule->state != 1) {
                flag = 0;
                break;
            }
        }
        if (flag) {
            #ifdef DEBUG
            printf("Pushing %s\n", (char*)vector_get(sorted, i));
            #endif
            rulen->state = 2;
            queue_push(job_queue, vector_get(sorted, i));
        }
        vector_destroy(neighbors);
    }
    if(vector_size(sorted) == 0){
        queue_push(job_queue, "");
    }

    // Create threads
    #ifdef DEBUG
    printf("Creating threads\n");
    #endif
    pthread_t threads[num_threads];
    struct thread_data data[num_threads];
    pthread_mutex_init(&lock, NULL);
    for(size_t i = 0; i < num_threads; i++){
        data[i].q = job_queue;
        data[i].dependency_graph = dependency_graph;
        data[i].sorted = sorted;
        pthread_create(&threads[i], NULL, thread_make, &data[i]);
    }
    for(size_t i = 0; i < num_threads; i++){
        pthread_join(threads[i], NULL);
    }

    // Clean up
    queue_destroy(job_queue);
    graph_destroy(dependency_graph);
    vector_destroy(vertices);
    vector_destroy(sorted);
    pthread_mutex_destroy(&lock);

    return 0;
}
