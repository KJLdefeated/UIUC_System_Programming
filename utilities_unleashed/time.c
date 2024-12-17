/**
 * utilities_unleashed
 * CS 341 - Fall 2024
 */

#include "format.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_time_usage();
    }

    
    pid_t pid = fork();
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    if (pid < 0) {
        print_fork_failed();
    }
    else if (pid == 0) {
        
        if (execvp(argv[1], argv + 1) < 0) {
            print_exec_failed();
        }
        exit(0);
        
    }
    else {
        wait(NULL);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double duration = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    display_results(argv, duration);
    return 0;
}
