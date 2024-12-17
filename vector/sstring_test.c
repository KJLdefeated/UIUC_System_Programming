/**
 * vector
 * CS 341 - Fall 2024
 */
#include "sstring.h"
#include <assert.h>
#include <string.h>

void test_cstr_to_sstring() {
    const char* test_str = "Hello, World!";
    sstring* str = cstr_to_sstring(test_str);
    assert(str != NULL);
    char* result = sstring_to_cstr(str);
    assert(strcmp(result, test_str) == 0);
    free(result);
    sstring_destroy(str);

    // Test with empty string
    str = cstr_to_sstring("");
    assert(str != NULL);
    result = sstring_to_cstr(str);
    assert(strcmp(result, "") == 0);
    free(result);
    sstring_destroy(str);

    // Test with NULL
    str = cstr_to_sstring(NULL);
    assert(str == NULL);
}

void test_sstring_append() {
    sstring* str1 = cstr_to_sstring("Hello, ");
    sstring* str2 = cstr_to_sstring("World!");
    int len = sstring_append(str1, str2);
    assert(len == 13);
    char* result = sstring_to_cstr(str1);
    assert(strcmp(result, "Hello, World!") == 0);
    free(result);
    sstring_destroy(str1);
    sstring_destroy(str2);

    // Test appending empty string
    str1 = cstr_to_sstring("Test");
    str2 = cstr_to_sstring("");
    len = sstring_append(str1, str2);
    assert(len == 4);
    result = sstring_to_cstr(str1);
    assert(strcmp(result, "Test") == 0);
    free(result);
    sstring_destroy(str1);
    sstring_destroy(str2);
}

void test_sstring_split() {
    sstring* str = cstr_to_sstring("apple,banana,cherry");
    vector* result = sstring_split(str, ',');
    assert(vector_size(result) == 3);
    assert(strcmp(vector_get(result, 0), "apple") == 0);
    assert(strcmp(vector_get(result, 1), "banana") == 0);
    assert(strcmp(vector_get(result, 2), "cherry") == 0);
    vector_destroy(result);
    sstring_destroy(str);

    // Test with no delimiter
    str = cstr_to_sstring("hello");
    result = sstring_split(str, ',');
    assert(vector_size(result) == 1);
    assert(strcmp(vector_get(result, 0), "hello") == 0);
    vector_destroy(result);
    sstring_destroy(str);

    // Test with empty string
    str = cstr_to_sstring("");
    result = sstring_split(str, ',');
    assert(vector_size(result) == 0);
    vector_destroy(result);

    // Test sstring_split where the result have empty strings.
    str = cstr_to_sstring("ee");
    result = sstring_split(str, 'e');
    assert(vector_size(result) == 3);
    assert(strcmp(vector_get(result, 0), "") == 0);
    assert(strcmp(vector_get(result, 1), "") == 0);
    assert(strcmp(vector_get(result, 2), "") == 0);
    vector_destroy(result);

    sstring_destroy(str);
}

void test_sstring_substitute() {
    sstring* str = cstr_to_sstring("The quick brown fox");
    int result = sstring_substitute(str, 0, "quick", "slow");
    assert(result == 0);
    char* cstr = sstring_to_cstr(str);
    assert(strcmp(cstr, "The slow brown fox") == 0);
    free(cstr);
    
    // Test substitution not found
    result = sstring_substitute(str, 0, "black", "white");
    assert(result == -1);
    
    // Test with offset
    result = sstring_substitute(str, 9, "brown", "red");
    assert(result == 0);
    cstr = sstring_to_cstr(str);
    assert(strcmp(cstr, "The slow red fox") == 0);
    free(cstr);
    
    sstring_destroy(str);
}

void test_sstring_slice() {
    sstring* str = cstr_to_sstring("Hello, World!");
    char* slice = sstring_slice(str, 0, 5);
    assert(strcmp(slice, "Hello") == 0);
    free(slice);

    slice = sstring_slice(str, 7, 12);
    assert(strcmp(slice, "World") == 0);
    free(slice);

    // Test empty slice
    slice = sstring_slice(str, 5, 5);
    assert(strcmp(slice, "") == 0);
    free(slice);

    sstring_destroy(str);
}


int main(int argc, char *argv[]) {
    // TODO create some tests
    test_cstr_to_sstring();
    test_sstring_append();
    test_sstring_split();
    test_sstring_substitute();
    test_sstring_slice();

    printf("All tests passed!\n");

    return 0;
}
