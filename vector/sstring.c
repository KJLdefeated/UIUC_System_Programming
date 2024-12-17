/**
 * vector
 * CS 341 - Fall 2024
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    char* str;
    size_t size;
};

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    if (!input) return NULL;
    sstring* s = malloc(sizeof(sstring));
    s->str = strdup(input);
    s->size = strlen(input);
    return s;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    return input ? strdup(input->str) : NULL;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    if (this == NULL || addition == NULL) return -1;
    size_t new_size = this->size + addition->size;
    char* new_str = realloc(this->str, new_size + 1);
    if (!new_str) return -1;
    this->str = new_str;
    strcat(this->str, addition->str);
    this->size = new_size;
    return new_size;
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    if (!this) return NULL;
    vector* v = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    if (!v) return NULL;
    char* start = this->str;
    char* end = this->str;
    int f = 0;
    while (*end) {
        if (*end == delimiter) {
            *end = '\0';
            vector_push_back(v, start);
            start = end + 1;
            f = 1;
        }
        else f = 0;
        end++;
    }
    if (f || start != end) vector_push_back(v, start);
    return v;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    if (!this || !target || !substitution) return -1;
    char *pos = strstr(this->str + offset, target);
    if (!pos) return -1;
    
    size_t target_len = strlen(target);
    size_t sub_len = strlen(substitution);
    size_t new_size = this->size - target_len + sub_len;

    char* new_str = malloc(new_size + 1);
    if (!new_str) return -1;

    strncpy(new_str, this->str, pos - this->str);
    strcpy(new_str + (pos - this->str), substitution);
    strcpy(new_str + (pos - this->str) + sub_len, pos + target_len);
    free(this->str);
    this->str = new_str;
    this->size = new_size;
    return 0;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    if (!this) return NULL;
    if (start < 0 || end < 0 || start > end || end > (int)this->size) return NULL;
    char* new_str = malloc(end - start + 1);
    if (!new_str) return NULL;
    strncpy(new_str, this->str + start, end - start);
    new_str[end - start] = '\0';
    return new_str;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    if (!this) return;
    free(this->str);
    free(this);
}
