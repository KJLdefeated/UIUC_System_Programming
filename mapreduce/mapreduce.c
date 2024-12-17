/**
 * mapreduce
 * CS 341 - Fall 2024
 */
#include "utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int fork_process(char** exec_args, int* input_fd, int* output_fd, int output_file_fd) {
    int pid = fork();
    if (pid == -1) {
        // perror("fork");
        return 1;
    }
    if (pid == 0) {
        // Redirect input and output.
        if (input_fd) {
            dup2(input_fd[0], 0);
            close(input_fd[0]);
            close(input_fd[1]);
        }
        if (output_fd) {
            dup2(output_fd[1], 1);
            close(output_fd[0]);
            close(output_fd[1]);
        }
        if (output_file_fd != -1) {
            dup2(output_file_fd, 1);
            close(output_file_fd);
        }

        // Execute the program.
        if (execvp(exec_args[0], exec_args) == -1) {
            // perror("execvp");
            exit(1);
        }
        exit(0);
    }
    return pid;
}

int main(int argc, char **argv) {
    if (argc != 6) {
        print_usage();
        return 1;
    }
    char* input_file = argv[1];
    char* output_file = argv[2];
    char* mapper_exec = argv[3];
    char* reducer_exec = argv[4];
    int num_mappers = atoi(argv[5]);

    // Create an input pipe for each mapper.
    int input_pipes[num_mappers][2]; 
    for (int i = 0; i < num_mappers; i++) {
        if (pipe(input_pipes[i]) == -1) {
            perror("pipe");
            return 1;
        }
    }

    // Create one input pipe for the reducer.
    int reducer_input_pipe[2];
    if (pipe(reducer_input_pipe) == -1) {
        perror("pipe");
        return 1;
    }

    // Open the output file.
    int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (output_fd == -1) {
        perror("open output file");
        exit(1);
    }

    int splitter_pid[num_mappers];
    int mapper_pid[num_mappers];
    int reducer_pid;

    // Start a splitter process for each mapper.
    for(int i=0;i<num_mappers;i++){
        char idx[10], n_mp[10];
        sprintf(idx, "%d", i);
        sprintf(n_mp, "%d", num_mappers);
        char* exec_args[] = {"./splitter", input_file, n_mp, idx, NULL};
        
        splitter_pid[i] = fork_process(exec_args, NULL, input_pipes[i], -1);
    }

    for(int i=0;i<num_mappers;i++){
        int status;
        waitpid(splitter_pid[i], &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            print_nonzero_exit_status("splitter", WEXITSTATUS(status));
        }
    }

    // Start all the mapper processes.
    for(int i=0;i<num_mappers;i++){
        char* exec_args[] = {mapper_exec, NULL};
        mapper_pid[i] = fork_process(exec_args, input_pipes[i], reducer_input_pipe, -1);
        close(input_pipes[i][0]);
        close(input_pipes[i][1]);
    }

    for (int i = 0; i < num_mappers; i++) {
        int status;
        waitpid(mapper_pid[i], &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            print_nonzero_exit_status(mapper_exec, WEXITSTATUS(status));
        }
    }

    close(reducer_input_pipe[1]);

    // Start the reducer process.
    char* exec_args[] = {reducer_exec, NULL};
    reducer_pid = fork_process(exec_args, reducer_input_pipe, NULL, output_fd);

    // Wait for the reducer to finish.
    int status;
    waitpid(reducer_pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        print_nonzero_exit_status(reducer_exec, WEXITSTATUS(status));
    }

    // Count the number of lines in the output file.
    print_num_lines(output_file);
    return 0;
}
