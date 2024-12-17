/**
 * nonstop_networking
 * CS 341 - Fall 2024
 */
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <netdb.h>

#include "vector.h"
#include "common.h"

char **parse_args(int argc, char **argv);
verb check_args(char **args);
int connect_to_server(char *host, char *port);
struct Cli_Request *create_request(verb method, char *remote, char *local);
int send_to_server(int sockfd, struct Cli_Request *req, char *local);
int get_from_server(int sockfd, struct Cli_Request *req, char *local);
int main(int argc, char **argv) {
    char **args = parse_args(argc, argv);
    if (args == NULL) {
        print_client_usage();
        return 1;
    }
    
    char* host = args[0];
    char* port = args[1];
    verb method = check_args(args);
    char* remote = (method == LIST ? NULL : args[3]);
    char* local = (method == LIST ? NULL : args[4]);
    
    int sockfd = connect_to_server(host, port);
    
    struct Cli_Request *req = create_request(method, remote, local);
    
    if (send_to_server(sockfd, req, local) != 0) {
        return 1;
    }

    if (shutdown(sockfd, SHUT_WR) == -1) {
        perror("shutdown failed");
        close(sockfd);
        return 1;
    }

    if (get_from_server(sockfd, req, local) != 0) {
        return 1;
    }

    free(req);
    free(args);
    close(sockfd);

    return 0;
}

int get_from_server(int sockfd, struct Cli_Request *req, char *local) {
    char buffer[DATA_SIZE];
    memset(buffer, 0, DATA_SIZE);
    if (recvall(sockfd, buffer, 1) < 0) {
        perror("recvall");
        return 1;
    }
    if (buffer[0] == 'O') {
        recvall(sockfd, buffer + 1, 2);
        if (strcmp(buffer, "OK\n") != 0) {
            print_invalid_response();
            return 1;
        }
        if (req->method == GET) {
            size_t size = 0;
            if (recvall(sockfd, &size, sizeof(size_t)) < 0) {
                perror("recvall");
                return 1;
            }
            printf("Size: %zu\n", size);
            size_t bytes_received = 0;

            FILE *file = fopen(local, "wb");
            if (file == NULL) {
                perror("fopen");
                return 1;
            }

            char buffer[DATA_SIZE];
            while (1) {
                // size_t bytes_to_read = size - bytes_received > DATA_SIZE ? DATA_SIZE : size - bytes_received;
                int tot = recvall(sockfd, buffer, DATA_SIZE);
                if (tot < 0) {
                    perror("recvall");
                    return 1;
                }
                if (tot == 0) {
                    break;
                }
                fwrite(buffer, 1, tot, file);
                bytes_received += tot;
            }
            
            fclose(file);

            printf("Received %zu bytes\n", bytes_received);

            if (bytes_received < size) {
                if (req->method == GET) {
                    remove(local);
                }
                print_too_little_data();
                return 1;
            }
            else if (bytes_received > size) {
                if (req->method == GET) {
                    remove(local);
                }
                print_received_too_much_data();
                return 1;
            }
        }
        else if (req->method == LIST) {
            size_t size = 0;
            if (recvall(sockfd, &size, sizeof(size_t)) < 0) {
                perror("recvall");
                return 1;
            }
            // printf("Size: %zu\n", size);
            size_t bytes_received = 0;

            char buffer[DATA_SIZE];
            char *output = malloc(size + 1);
            while (1) {
                // size_t bytes_to_read = size - bytes_received > DATA_SIZE ? DATA_SIZE : size - bytes_received;
                int tot = recvall(sockfd, buffer, DATA_SIZE);
                if (tot < 0) {
                    perror("recvall");
                    return 1;
                }
                if (tot == 0) {
                    break;
                }
                bytes_received += tot;
                if (bytes_received > size) {
                    print_received_too_much_data();
                    free(output);
                    return 1;
                }
                // printf("%s", buffer);
                memcpy(output + bytes_received - tot, buffer, tot);
            }
            
            if (bytes_received < size) {
                print_too_little_data();
                free(output);
                return 1;
            }

            output[size] = '\0';
            printf("%s", output);
            free(output);
        }
        else {
            print_success();
        }
    }
    else if (buffer[0] == 'E') {
        recvall(sockfd, buffer + 1, 5);
        char error[256];
        memset(error, 0, 256);
        if (strcmp(buffer, "ERROR\n") != 0) {
            print_invalid_response();
            return 1;
        }
        while(recvall(sockfd, buffer, 1) > 0) {
            if (buffer[0] == '\n') {
                break;
            }
            strncat(error, buffer, 1);
        }
        print_error_message(error);
        return 1;
    }
    else {
        print_invalid_response();
        return 1;
    }
    return 0;
}

int send_to_server(int sockfd, struct Cli_Request *req, char *local){
    char buffer[DATA_SIZE];
    memset(buffer, 0, DATA_SIZE);
    if (req->method == GET) {
        snprintf(buffer, DATA_SIZE, "GET %s\n", req->filename);
        if (sendall(sockfd, buffer, strlen(buffer)) < 0) {
            perror("sendall");
            return 1;
        }
    }
    else if (req->method == PUT) {
        snprintf(buffer, DATA_SIZE, "PUT %s\n", req->filename);
        if (sendall(sockfd, buffer, strlen(buffer)) < 0) {
            perror("sendall");
            return 1;
        }
        size_t size = req->size;
        if (sendall(sockfd, &size, sizeof(size_t)) < 0) {
            perror("sendall");
            return 1;
        }
        FILE *file = fopen(local, "rb");
        if (file == NULL) {
            perror("fopen");
            return 1;
        }
        while (1) {
            int bytes_read = fread(buffer, 1, DATA_SIZE, file);
            if (bytes_read <= 0) {
                break;
            }
            if (sendall(sockfd, buffer, bytes_read) < 0) {
                perror("sendall");
                return 1;
            }
        }
        fclose(file);
    }
    else if (req->method == DELETE) {
        snprintf(buffer, DATA_SIZE, "DELETE %s\n", req->filename);
        if (sendall(sockfd, buffer, strlen(buffer)) < 0) {
            perror("sendall");
            return 1;
        }
    }
    else if (req->method == LIST) {
        snprintf(buffer, DATA_SIZE, "LIST\n");
        if (sendall(sockfd, buffer, strlen(buffer)) < 0) {
            perror("sendall");
            return 1;
        }
    }
    else {
        return 1;
    }
    return 0;
}

struct Cli_Request *create_request(verb method, char *remote, char *local) {
    struct Cli_Request *req = malloc(sizeof(struct Cli_Request));
    req->method = method;
    req->special = '\0';
    if (remote != NULL) {
        strncpy(req->filename, remote, MAX_FILENAME);
    }
    req->size = 0;
    if (local != NULL && method == PUT) {
        FILE *file = fopen(local, "rb");
        if (file == NULL) {
            perror("fopen");
            exit(1);
        }
        fseek(file, 0, SEEK_END);
        req->size = ftell(file);
        fseek(file, 0, SEEK_SET);
        fclose(file);
    }
    return req;
}

int connect_to_server(char *host, char *port) {
    struct addrinfo *server_info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(host, port, &hints, &server_info) != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(errno));
        exit(1);
    }
    int sockfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }
    if (connect(sockfd, server_info->ai_addr, server_info->ai_addrlen) < 0) {
        perror("connect");
        exit(1);
    }
    freeaddrinfo(server_info);
    return sockfd;
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
