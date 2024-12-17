/**
 * vector
 * CS 341 - Fall 2024
 */
#include "vector.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

void* allocate_and_initialize(const char* type, size_t count) {
    void* data = NULL;
    if (strcmp(type, "char") == 0) {
        char* arr = malloc(count * sizeof(char));
        for (size_t i = 0; i < count; i++) arr[i] = 'a' + i;
        data = arr;
    } else if (strcmp(type, "int") == 0) {
        int* arr = malloc(count * sizeof(int));
        for (size_t i = 0; i < count; i++) arr[i] = i + 1;
        data = arr;
    } else if (strcmp(type, "str") == 0) {
        char** arr = malloc(count * sizeof(char*));
        for (size_t i = 0; i < count; i++) {
            arr[i] = malloc(4 * sizeof(char));
            snprintf(arr[i], 4, "%c%c%c", (char)('a'+i), (char)('b'+i), (char)('c'+i));
        }
        data = arr;
    } else if (strcmp(type, "double") == 0) {
        double* arr = malloc(count * sizeof(double));
        for (size_t i = 0; i < count; i++) arr[i] = (i + 1) * 1.1;
        data = arr;
    } else if (strcmp(type, "float") == 0) {
        float* arr = malloc(count * sizeof(float));
        for (size_t i = 0; i < count; i++) arr[i] = (i + 1) * 1.1f;
        data = arr;
    } else if (strcmp(type, "long") == 0) {
        long* arr = malloc(count * sizeof(long));
        for (size_t i = 0; i < count; i++) arr[i] = (i + 1) * 10000L;
        data = arr;
    } else if (strcmp(type, "short") == 0) {
        short* arr = malloc(count * sizeof(short));
        for (size_t i = 0; i < count; i++) arr[i] = i + 1;
        data = arr;
    } else if (strcmp(type, "unsigned char") == 0) {
        unsigned char* arr = malloc(count * sizeof(unsigned char));
        for (size_t i = 0; i < count; i++) arr[i] = i + 1;
        data = arr;
    } else if (strcmp(type, "unsigned int") == 0) {
        unsigned int* arr = malloc(count * sizeof(unsigned int));
        for (size_t i = 0; i < count; i++) arr[i] = i + 1;
        data = arr;
    } else if (strcmp(type, "unsigned long") == 0) {
        unsigned long* arr = malloc(count * sizeof(unsigned long));
        for (size_t i = 0; i < count; i++) arr[i] = i + 1;
        data = arr;
    } else if (strcmp(type, "unsigned short") == 0) {
        unsigned short* arr = malloc(count * sizeof(unsigned short));
        for (size_t i = 0; i < count; i++) arr[i] = i + 1;
        data = arr;
    }
    return data;
}

void free_data(const char* type, void* data, size_t count) {
    if (strcmp(type, "str") == 0) {
        char** arr = (char**)data;
        for (size_t i = 0; i < count; i++) {
            free(arr[i]);
        }
    }
    free(data);
}

void check(const char* dtype, void** a, void** b){
    if (strcmp(dtype, "char") == 0) assert(*(char *)a == *(char *)b);
    if (strcmp(dtype, "int") == 0) assert(*(int *)a == *(int *)b);
    if (strcmp(dtype, "str") == 0) assert(strcmp(*(char **)a, *(char **)b) == 0);
    if (strcmp(dtype, "double") == 0) assert(*(double *)a == *(double *)b);
    if (strcmp(dtype, "float") == 0) assert(*(float *)a == *(float *)b);
    if (strcmp(dtype, "long") == 0) assert(*(long *)a == *(long *)b);
    if (strcmp(dtype, "short") == 0) assert(*(short *)a == *(short *)b);
    if (strcmp(dtype, "unsigned char") == 0) assert(*(unsigned char *)a == *(unsigned char *)b);
    if (strcmp(dtype, "unsigned int") == 0) assert(*(unsigned int *)a == *(unsigned int *)b);
    if (strcmp(dtype, "unsigned long") == 0) assert(*(unsigned long *)a == *(unsigned long *)b);
    if (strcmp(dtype, "unsigned short") == 0) assert(*(unsigned short *)a == *(unsigned short *)b);
}

void unit_test_per_type(const char* dtype, void** data){
    // Test vector creation
    vector* v = NULL;
    if (strcmp(dtype, "char") == 0) v = char_vector_create();
    if (strcmp(dtype, "int") == 0) v = int_vector_create();
    if (strcmp(dtype, "str") == 0) v = string_vector_create();
    if (strcmp(dtype, "double") == 0) v = double_vector_create();
    if (strcmp(dtype, "float") == 0) v = float_vector_create();
    if (strcmp(dtype, "long") == 0) v = long_vector_create();
    if (strcmp(dtype, "short") == 0) v = short_vector_create();
    if (strcmp(dtype, "unsigned char") == 0) v = unsigned_char_vector_create();
    if (strcmp(dtype, "unsigned int") == 0) v = unsigned_int_vector_create();
    if (strcmp(dtype, "unsigned long") == 0) v = unsigned_long_vector_create();
    if (strcmp(dtype, "unsigned short") == 0) v = unsigned_short_vector_create();

    assert(v);
    assert(vector_size(v) == 0);
    
    // Check push back
    vector_push_back(v, &data[0]);
    assert(vector_size(v) == 1);
    check(dtype, vector_get(v, 0), &data[0]);
    vector_push_back(v, &data[1]);
    assert(vector_size(v) == 2);
    check(dtype, vector_get(v, 1), &data[1]);

    // Check front, begin, back, end
    check(dtype, vector_front(v), &data[0]);
    check(dtype, *vector_begin(v), &data[0]);
    check(dtype, vector_back(v), &data[1]);
    // check(dtype, *vector_end(v), &data[1]);

    // Check pop back
    vector_pop_back(v);
    assert(vector_size(v) == 1);
    check(dtype, vector_get(v, 0), &data[0]);
    vector_pop_back(v);
    assert(vector_size(v) == 0);
    
    // Check begin, end
    assert(*vector_begin(v) == NULL);
    assert(*vector_end(v) == NULL);

    // Check insert
    vector_push_back(v, &data[0]);
    vector_push_back(v, &data[2]);
    vector_insert(v, 1, &data[1]);
    assert(vector_size(v) == 3);
    check(dtype, vector_get(v, 0), &data[0]);
    check(dtype, vector_get(v, 1), &data[1]);
    check(dtype, vector_get(v, 2), &data[2]);

    // Check at
    check(dtype, *vector_at(v, 0), &data[0]);
    check(dtype, *vector_at(v, 1), &data[1]);
    check(dtype, *vector_at(v, 2), &data[2]);

    // Check set
    vector_set(v, 0, &data[3]);
    check(dtype, vector_get(v, 0), &data[3]);
    vector_set(v, 1, &data[4]);
    check(dtype, vector_get(v, 1), &data[4]);
    vector_set(v, 2, &data[5]);
    check(dtype, vector_get(v, 2), &data[5]);

    // Check erase
    vector_erase(v, 1);
    assert(vector_size(v) == 2);
    check(dtype, vector_get(v, 0), &data[3]);
    check(dtype, vector_get(v, 1), &data[5]);
    
    // Check clear
    vector_clear(v);
    assert(vector_size(v) == 0);
    assert(*vector_begin(v) == NULL);
    assert(*vector_end(v) == NULL);

    // Check reserve
    vector_reserve(v, 10);
    assert(vector_capacity(v) == 16);
    vector_reserve(v, 2);
    assert(vector_capacity(v) == 16);

    // Check destructor
    vector_destroy(v);
}

bool stress_test(){
    vector* v = char_vector_create();
    for (int i = 0; i < 1000000; i++){
        char* a = malloc(sizeof(char));
        *a = 'a';
        vector_push_back(v, a);
    }
    for (int i = 0; i < 1000000; i++){
        assert(*(char *)vector_get(v, i) == 'a');
    }
    vector_clear(v);
    assert(vector_size(v) == 0);
    assert(*vector_begin(v) == NULL);
    assert(*vector_end(v) == NULL);
    vector_destroy(v);
    return true;
}

int main(int argc, char *argv[]) {
    // Write your test cases here
    // Test shallow 
    vector* v1 = vector_create(shallow_copy_constructor, shallow_destructor, shallow_default_constructor);
    assert(v1 != NULL);
    assert(vector_size(v1) == 0);
    assert(vector_capacity(v1) == INITIAL_CAPACITY);
    assert(vector_begin(v1) != NULL);
    vector_destroy(v1);
    
    vector* v2 = shallow_vector_create();
    assert(v2 != NULL);
    assert(vector_size(v2) == 0);
    assert(vector_capacity(v2) == INITIAL_CAPACITY);
    assert(vector_begin(v2) != NULL);
    vector_destroy(v2);

    // Test vector by type
    void* data1 = allocate_and_initialize("char", 6);
    unit_test_per_type("char", &data1);
    free_data("char", data1, 6);

    void* data2 = allocate_and_initialize("int", 6);
    unit_test_per_type("int", &data2);
    free_data("int", data2, 6);

    void* data3 = allocate_and_initialize("str", 6);
    unit_test_per_type("str", &data3);
    free_data("str", data3, 6);

    void* data4 = allocate_and_initialize("double", 6);
    unit_test_per_type("double", &data4);
    free_data("double", data4, 6);

    void* data5 = allocate_and_initialize("float", 6);
    unit_test_per_type("float", &data5);
    free_data("float", data5, 6);

    void* data6 = allocate_and_initialize("long", 6);
    unit_test_per_type("long", &data6);
    free_data("long", data6, 6);

    void* data7 = allocate_and_initialize("short", 6);
    unit_test_per_type("short", &data7);
    free_data("short", data7, 6);

    void* data8 = allocate_and_initialize("unsigned char", 6);
    unit_test_per_type("unsigned char", &data8);
    free_data("unsigned char", data8, 6);

    void* data9 = allocate_and_initialize("unsigned int", 6);
    unit_test_per_type("unsigned int", &data9);
    free_data("unsigned int", data9, 6);

    void* data10 = allocate_and_initialize("unsigned long", 6);
    unit_test_per_type("unsigned long", &data10);
    free_data("unsigned long", data10, 6);

    void* data11 = allocate_and_initialize("unsigned short", 6);
    unit_test_per_type("unsigned short", &data11);
    free_data("unsigned short", data11, 6);

    // Stress test
    bool stress_result = stress_test();
    assert(stress_result);
    
    printf("All test cases pass\n");
    
    return 0;
}
