/**
 * charming_chatroom
 * CS 341 - Fall 2024
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here
    ssize_t hsize = htonl(size);
    int bytes_written = write_all_to_socket(socket, (char *)&hsize, MESSAGE_SIZE_DIGITS);
    if (bytes_written == -1 || bytes_written == 0) {
        return bytes_written;
    }
    return bytes_written;
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    size_t bytes_read = 0;
    while (bytes_read < count) {
        ssize_t read_bytes = recv(socket, buffer + bytes_read, count - bytes_read, 0);
        if (read_bytes == 0) {
            return bytes_read;
        }
        if (read_bytes == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        bytes_read += read_bytes;
    }
    return bytes_read;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    size_t bytes_written = 0;
    while (bytes_written < count) {
        ssize_t write_bytes = send(socket, buffer + bytes_written, count - bytes_written, 0);
        if (write_bytes == 0) {
            return bytes_written;
        }
        if (write_bytes == -1) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        bytes_written += write_bytes;
    }
    return bytes_written;
}
