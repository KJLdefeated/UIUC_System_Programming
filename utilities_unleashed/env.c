/**
 * utilities_unleashed
 * CS 341 - Fall 2024
 */
#include "format.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_env_usage();
    }
    int flag = 1;
    for (int i=1; i<argc; i++) {
        if (strcmp(argv[i], "--") == 0 && i != argc-1) {
            flag = 0;
            break;
        }
    }
    if (flag) {
        print_env_usage();
    }
    int i;
    for (i=1;i<argc;i++) {
        if (strcmp(argv[i], "--") == 0) {
            break;
        }
        if (strchr(argv[i], '=') == NULL) {
            print_env_usage();
        }
        // separate key and value
        char *key = strtok(argv[i], "=");
        char *value = strtok(NULL, "=");
        if (value[0] == '%'){
            char *env = getenv(value+1);
            if (env == NULL) {
                print_env_usage();
            }
            value = env;
        }
        if (setenv(key, value, 1) < 0) {
            print_environment_change_failed();
        }
    }
    pid_t pid = fork();
    if (pid < 0) {
        print_fork_failed();
    }
    else if (pid == 0) {
        if (execvp(argv[i+1], argv+i+1) < 0) {
            print_exec_failed();
        }
    }
    else {
        wait(NULL);
    }
    
    return 0;
}
