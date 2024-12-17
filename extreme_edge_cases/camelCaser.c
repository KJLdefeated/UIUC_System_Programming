/**
 * extreme_edge_cases
 * CS 341 - Fall 2024
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    if (input_str == NULL) return NULL;
    int input_len = strlen(input_str);
    char* sentence = malloc(input_len * sizeof(char));
    const char *start = input_str;
    const char *end = start;
    int cnt = 0;

    // Count how many punctuations in strings
    while(*start != '\0'){
        end = start;
        while (*end != '\0' && !ispunct(*end)) {
            end++;
        }
        if (*end == '\0') {
            break;
        }
        end++;
        start = end;
        cnt++;
    }
    
    // Calculate the result
    char** result = malloc((cnt+1) * sizeof(char*));
    start = input_str;
    end = input_str;
    for(int i=0;i<cnt;i++){
        end = start;
        while (*end != '\0' && !ispunct(*end)) {
            end++;
        }
        if (*end == '\0' && start == end) {
            break;
        }
        int length = end - start;
        strncpy(sentence, start, length);
        sentence[length] = '\0';
        //printf("%s\n", sentence);
        result[i] = malloc((length+1) * sizeof(char));
        int k = 0, Cap = 0;
        for(int j=0;j<length;j++){
            if(isalpha(sentence[j])){
                if (k == 0){
                    result[i][k] = tolower(sentence[j]);
                    Cap = 0;
                    k++;
                }
                else if(Cap) {
                    result[i][k] = toupper(sentence[j]);
                    Cap = 0;
                    k++;
                }
                else {
                    result[i][k] = tolower(sentence[j]);
                    k++;    
                }
            }
            else if (isspace(sentence[j])){
                Cap = 1;
            }
            else{
                result[i][k] = sentence[j];
                k++;
            }
        }
        result[i][k] = '\0';
        start = end + 1;
        while (isspace(*start)) start++;
    }
    free(sentence);
    result[cnt] = NULL;
    return result;
}

void destroy(char **result) {
    // TODO: Implement me!
    if (result == NULL) return;
    for(int i=0;result[i] != NULL;i++){
        free(result[i]);
    }
    free(result);
}
