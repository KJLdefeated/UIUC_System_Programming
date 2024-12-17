/**
 * mini_memcheck
 * CS 341 - Fall 2024
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Your tests here using malloc and free
    // All tests for this Lab:
    // bad_free_test: Test with a program that frees or reallocs invalid addresses
    // calloc_test: Test calloc with a program that has no memory leaks
    // full_test: Test with a program that has a variety of issues
    // leak_test: Test with a program that has memory leaks
    // no_leak_test: Test with a program that has no memory leaks
    // realloc_test: Test realloc with a program that has no memory leaks

    // bad_free_test
    char *ptr = (char *)malloc(10);
    free(ptr + 1);
    free(ptr);

    // // calloc_test
    // char *ptr1 = (char *)calloc(10, sizeof(char));
    // free(ptr1);

    // // full_test
    // char *ptr2 = (char *)malloc(10);
    // free(ptr2);
    // ptr2 = (char *)malloc(10);
    // ptr2 = (char *)realloc(ptr2, 20);
    // free(ptr2);

    // // leak_test
    // void *p1 = malloc(30);
    // void *p2 = malloc(40);
    // void *p3 = malloc(50);
    // free(p2);

    // // no_leak_test
    // char *ptr4 = (char *)malloc(10);
    // free(ptr4);

    // // realloc_test
    // char *ptr5 = (char *)malloc(10);
    // ptr5 = (char *)realloc(ptr5, 20);
    // free(ptr5);
    return 0;
}