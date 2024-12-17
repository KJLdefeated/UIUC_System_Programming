/**
 * deepfried_dd
 * CS 341 - Fall 2024
 */
#include "format.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

static size_t full_blocks_in = 0;
static size_t partial_blocks_in = 0;
static size_t full_blocks_out = 0;
static size_t partial_blocks_out = 0;
static size_t total_bytes_copied = 0;
static double time_elapsed = 0;
static clock_t start = -1;
static clock_t end = -1;

// signal handler
void signal_handler(int signum) {
    if(signum == SIGUSR1) {
        if (start == -1) time_elapsed = 0;
        else if (end == -1) time_elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
        else time_elapsed = (double)(end - start) / CLOCKS_PER_SEC;
        print_status_report(full_blocks_in, partial_blocks_in, full_blocks_out, partial_blocks_out, total_bytes_copied, time_elapsed);
        exit(0);
    }
}

int main(int argc, char **argv) {
    // Catch SIGUSR1
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);
    FILE *input = stdin;
    FILE *output = stdout;
    size_t block_size = 512;
    int total_blocks = -1; // Entire file
    int skip_input_blocks = 0;
    int skip_output_blocks = 0;
    int c;
    while((c = getopt(argc, argv, "i:o:b:c:p:k:")) != -1) {
        switch(c) {
            case 'i':
                input = fopen(optarg, "r");
                if(input == NULL) {
                    print_invalid_input(optarg);
                    return 1;
                }
                break;
            case 'o':
                output = fopen(optarg, "w");
                if(output == NULL) {
                    print_invalid_output(optarg);
                    return 1;
                }
                break;
            case 'b':
                block_size = atoi(optarg);
                break;
            case 'c':
                total_blocks = atoi(optarg);
                break;
            case 'p':
                skip_input_blocks = atoi(optarg);
                break;
            case 'k':
                skip_output_blocks = atoi(optarg);
                break;
            default:
                return 1;
        }
    }

    // Skip input blocks
    if(skip_input_blocks > 0) {
        fseek(input, skip_input_blocks * block_size, SEEK_SET);
    }

    // Skip output blocks
    if(skip_output_blocks) {
        fseek(output, skip_output_blocks * block_size, SEEK_SET);
    }

    // Copy blocks
    char *buffer = malloc(block_size);
    start = clock();
    while (total_blocks != 0) {
        size_t byte_read = fread(buffer, 1, block_size, input);
        if (byte_read == 0) {
            break;
        }
        if (total_blocks > 0) {
            total_blocks--;
        }
        if (byte_read == block_size) {
            full_blocks_in++;
        } else {
            partial_blocks_in++;
        }
        
        size_t byte_written = fwrite(buffer, 1, byte_read, output);
        if (byte_written == block_size) {
            full_blocks_out++;
        } else {
            partial_blocks_out++;
        }
        total_bytes_copied += byte_written;
    }
    end = clock();
    time_elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    print_status_report(full_blocks_in, partial_blocks_in, full_blocks_out, partial_blocks_out, total_bytes_copied, time_elapsed);
    return 0;
}