/**
 * nonstop_networking
 * CS 341 - Fall 2024
 */
#include "common.h"

size_t sendall(int sockfd, const void *buf, size_t len) {
    size_t total = 0;
    size_t bytesleft = len;
    size_t n;

    while (total < len) {
        size_t try_to_send = bytesleft > DATA_SIZE ? DATA_SIZE : bytesleft;
        n = (size_t)send(sockfd, buf + total, try_to_send, 0);
        if (n == (size_t)-1) {
            break;
        }
        if (n != try_to_send) {
            continue;
        }
        total += n;
        bytesleft -= n;
    }

    return total;
}

size_t recvall(int sockfd, void *buf, size_t len) {
    size_t total = 0;
    size_t bytesleft = len;
    int n;

    while (total < len) {
        size_t try_to_recv = bytesleft > DATA_SIZE ? DATA_SIZE : bytesleft;
        n = recv(sockfd, buf + total, try_to_recv, 0);
        if (n <= 0) {
            break;
        }
        total += (size_t)n;
        bytesleft -= (size_t)n;
    }

    return total;
}