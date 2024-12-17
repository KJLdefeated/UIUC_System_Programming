/**
 * nonstop_networking
 * CS 341 - Fall 2024
 */
#pragma once
#include <stddef.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#define LOG(...)                      \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
    } while (0);

typedef enum { GET, PUT, DELETE, LIST, V_UNKNOWN } verb;

#define MAX_FILENAME 255
#define DATA_SIZE 1024
#define RESPONSE_SIZE 128

// Client Request Header
/*
VERB [filename]\n
[File size][Binary Data]
*/
struct Cli_Request {
    verb method;
    char special;
    char filename[MAX_FILENAME+1];
    size_t size;
};

// Server Response Header
/*
RESPONSE\n
[Error Message]\n
[File size][Binary Data]
*/
struct Srv_Response {
    char response[RESPONSE_SIZE];
    char error_message[RESPONSE_SIZE];
    size_t size;
};

// Wrapped send and recv
size_t sendall(int sockfd, const void *buf, size_t len);
size_t recvall(int sockfd, void *buf, size_t len);