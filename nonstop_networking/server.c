/**
 * nonstop_networking
 * CS 341 - Fall 2024
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <signal.h>
#include "common.h"
#include "format.h"
#include "vector.h"

#define MAX_EVENTS 64
#define MAX_DIRNAME 265

static char *temp_directory;
static int flag;
static struct epoll_event event;
static struct epoll_event events[MAX_EVENTS];
static int serverSocket;
static int epollfd;

// Client state enumeration
typedef enum {
    READING_VERB,
    READING_FILENAME,
    READING_SIZE,
    READING_DATA,
    WRITING_RESPONSE,
    COMPLETE
} ClientState;

struct Client_info {
    int fd;
    verb method;
    char filename[MAX_DIRNAME];
    FILE* fp;
    size_t file_offset;
    ClientState state;
    char buffer[DATA_SIZE];
    size_t buffer_size;
    size_t boffset;
    size_t req_size;
    int error_type;
    char* response;
};

static int create_and_bind(char *port);
static int handle_new_connection(int serverSocket, int epollfd);
static void cleanup_client(int epoll_fd, struct Client_info *client);
static void handle_client_request(int epollfd, struct Client_info *client);
static int process_request(struct Client_info *client);
static char* list_response(struct Client_info *client, size_t *size);
static int send_get_response(struct Client_info *client);

void server_cleanup() {
    printf("Cleaning up...\n");
    close(serverSocket);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, serverSocket, NULL);
    for (int i = 0; i < MAX_EVENTS; i++) {
        if (events[i].data.ptr != NULL) {
            printf("Closing connection\n");
            struct Client_info *client = (struct Client_info *)events[i].data.ptr;
            // close(events[i].data.fd);
            epoll_ctl(epollfd, EPOLL_CTL_DEL, client->fd, NULL);
            close(client->fd);
            free(client);
            events[i].data.ptr = NULL;
        }
    }
    
    close(epollfd);

    // Remove content in temp directory
    DIR *dir = opendir(temp_directory);
    if (dir == NULL) {
        perror("opendir");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Regular file
            char filename[strlen(temp_directory) + strlen(entry->d_name) + 2];
            sprintf(filename, "%s/%s", temp_directory, entry->d_name);
            if (remove(filename) < 0) {
                perror("remove");
            }
        }
    }
    closedir(dir);
    if (rmdir(temp_directory) < 0) {
        perror("remove_directory");
        return;
    }    
}

void sigint_handler(int signum) {
    if (signum == SIGINT) {
        flag = 0;
        server_cleanup();
        exit(0);
    }
    if (signum == SIGPIPE) {
        // Do nothing
    }
}

void setup_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;  // Restart system calls if interrupted by handler
    
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_server_usage();
        return 1;
    }
    char *port = argv[1];

    // Create temp directory
    char tmp[] = "XXXXXX";
    temp_directory = mkdtemp(tmp);
    if (temp_directory == NULL) {
        perror("mktemp");
        return 1;
    }
    print_temp_directory(temp_directory);

    // Setup signal handler
    setup_signal_handler();
    
    serverSocket = create_and_bind(port);
    if (serverSocket < 0) {
        return 1;
    }

    if (listen(serverSocket, MAX_EVENTS) < 0) {
        perror("listen(): ");
        return 1;
    }

    epollfd = epoll_create1(0);
    if (epollfd < 0) {
        perror("epoll_create1(): ");
        return 1;
    }

    // event.events = EPOLLIN | EPOLLET;
    event.events = EPOLLIN;
    event.data.fd = serverSocket;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSocket, &event) < 0) {
        perror("epoll_ctl(): ");
        close(serverSocket);
        close(epollfd);
        return 1;
    }

    flag = 1;

    printf("Server started on port %s\n", port);

    set_nonblocking(serverSocket);

    while(flag) {
        // printf("Waiting for events...\n");
        int n = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (n < 0) {
            perror("epoll_wait(): ");
            break;
        }
        for (int i = 0; i < n; i++) {
            if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP) {
                if (events[i].data.fd == serverSocket) {
                    close(serverSocket);
                    close(epollfd);
                    return 1;
                }
                else {
                    printf("Closing connection\n");
                    struct Client_info *client = (struct Client_info *)events[i].data.ptr;
                    // close(events[i].data.fd);
                    cleanup_client(epollfd, client);
                    events[i].data.ptr = NULL;
                }
                continue;
            }
            if (events[i].data.fd == serverSocket) {
                handle_new_connection(serverSocket, epollfd);
                continue;
            }
            if (events[i].events & EPOLLIN) {
                struct Client_info *client = (struct Client_info *)events[i].data.ptr;
                handle_client_request(epollfd, client);
                set_nonblocking(client->fd);
            }
        }
    }

    // Cleanup
    return 0;
}

/*
Send GET data
First send OK\n<size>
Then send data
*/
static int send_get_response(struct Client_info *client) {
    assert(client->method == GET);
    assert(client->state == WRITING_RESPONSE);
    assert(client->response == NULL);
    FILE *file = client->fp;

    if (client->file_offset > 0) {
        fseek(file, client->file_offset, SEEK_SET);
    }
    else {
        // Send file size
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);
        client->req_size = size;
        
        sendall(client->fd, "OK\n", 3);
        sendall(client->fd, &size, sizeof(size_t));
    }
    
    size_t bytes_left = client->req_size - client->file_offset;
    size_t bytes_to_send = bytes_left > DATA_SIZE ? DATA_SIZE : bytes_left;

    char buffer[DATA_SIZE];
    size_t bytes_sent = fread(buffer, 1, bytes_to_send, file);
    sendall(client->fd, buffer, bytes_sent);
    client->file_offset += bytes_sent;
    

    if (client->file_offset >= client->req_size) {
        client->state = COMPLETE;
        fclose(file);
    }
    else {
        client->state = WRITING_RESPONSE;
    }

    // printf("(fd=%d) Sent %zu bytes\n", client->fd, bytes_sent);
    return 0;
}

// Construct list response return data
static char* list_response(struct Client_info *client, size_t *size) {
    assert(client->method == LIST);
    assert(client->state == WRITING_RESPONSE);
    assert(*size == 0);
    vector *files = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    DIR *dir = opendir(temp_directory);
    if (dir == NULL) {
        perror("opendir");
        return NULL;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Regular file
            vector_push_back(files, entry->d_name);
            *size += strlen(entry->d_name) + 1; // Include '\n'
        }
    }
    closedir(dir);
    char *data = malloc(*size);
    if (data == NULL) {
        perror("malloc");
        return NULL;
    }
    char *ptr = data;
    for (size_t i = 0; i < vector_size(files); i++) {
        char *file = (char*)vector_get(files, i);
        size_t len = strlen(file);
        memcpy(ptr, file, len);
        ptr += len;
        if (i != vector_size(files) - 1)
            *ptr = '\n';
        ptr++;
    }
    vector_destroy(files);
    // printf("List response: %s\n", data);
    // printf("Size: %zu\n", *size);
    return data;
}

static int process_request(struct Client_info *client) {
    if (client->state == READING_VERB) {
        if (strncmp(client->buffer, "GET", 3) == 0) {
            client->method = GET;
            client->state = READING_FILENAME;
            client->boffset = 4;
        }
        else if (strncmp(client->buffer, "PUT", 3) == 0) {
            client->method = PUT;
            client->state = READING_FILENAME;
            client->boffset = 4;
        }
        else if (strncmp(client->buffer, "DELETE", 6) == 0) {
            client->method = DELETE;
            client->state = READING_FILENAME;
            client->boffset = 7;
        }
        else if (strncmp(client->buffer, "LIST", 4) == 0) {
            client->method = LIST;
            client->state = WRITING_RESPONSE;
        }
        else {
            client->error_type = 0;
            client->state = WRITING_RESPONSE;
        }
    }
    else if (client->state == READING_FILENAME) {
        char* newline = strchr(client->buffer + client->boffset, '\n');
        if (newline == NULL) {
            // format error
            client->error_type = 0;
            client->state = WRITING_RESPONSE;
            return 0;
        }
        size_t len = newline - (client->buffer + client->boffset);
        memset(client->filename, 0, MAX_DIRNAME);
        memcpy(client->filename + strlen(temp_directory) + 1, client->buffer + client->boffset, len);
        memcpy(client->filename, temp_directory, strlen(temp_directory));
        client->filename[strlen(temp_directory)] = '/';
        client->file_offset = 0;
        client->boffset += len + 1;
        
        printf("Filename: %s\n", client->filename);

        if (client->method == PUT) {
            client->fp = fopen(client->filename, "wb");
            if (client->fp == NULL) {
                perror("fopen");
                return -1;
            }
        }
        else if (client->method == GET) {
            client->fp = fopen(client->filename, "rb");
            if (client->fp == NULL) {
                perror("fopen");
                client->error_type = 2;
                client->state = WRITING_RESPONSE;
                return 0;
            }
        }

        if (client->method == GET) {
            if (access(client->filename, F_OK) == -1) {
                client->error_type = 2;
                client->state = WRITING_RESPONSE;
            }
            else {
                client->state = WRITING_RESPONSE;
            }
        }
        else if (client->method == PUT) {
            client->state = READING_SIZE;
        }
        else if (client->method == DELETE) {
            if (access(client->filename, F_OK) == -1) {
                client->error_type = 2;
            }
            else {
                remove(client->filename);
            }
            client->state = WRITING_RESPONSE;
        }
    }
    else if (client->state == READING_SIZE) {
        assert(client->method == PUT);
        memcpy(&client->req_size, client->buffer + client->boffset, sizeof(size_t));
        client->boffset += sizeof(size_t);
        printf("Size: %zu\n", client->req_size);
        client->state = READING_DATA;
    }
    else if (client->state == READING_DATA) {
        // PUT will read and write to file
        // printf("Reading data\n");
        assert(client->method == PUT);
        // read data and write to file, copy on write
        FILE *file = client->fp;
        fseek(file, 0, SEEK_END);
        size_t bytes_left = client->req_size - client->file_offset; // bytes left to read
        size_t left_in_buffer = client->buffer_size - client->boffset;
        

        if (client->buffer_size == 0) {
            // No data in buffer
            client->state = WRITING_RESPONSE;
            client->error_type = 1;
            fclose(file);
            remove(client->filename);
            return 0;
        }

        size_t bytes_to_write = left_in_buffer > bytes_left ? bytes_left : left_in_buffer;
        fwrite(client->buffer + client->boffset, 1, bytes_to_write, file);
        
        bytes_left -= bytes_to_write;
        client->boffset = 0;
        client->buffer_size = 0;
        client->file_offset += bytes_to_write;
        left_in_buffer -= bytes_to_write;

        // printf("Buffer size: %zu\n", client->buffer_size);
        // printf("Bytes left: %zu\n", bytes_left);
        // printf("Left in buffer: %zu\n", left_in_buffer);

        // fclose(file);

        if (left_in_buffer > 0 && bytes_left <= 0) {
            // Too much data
            client->error_type = 1;
            client->state = WRITING_RESPONSE;
            remove(client->filename);
            fclose(file);
        }
        else if (client->file_offset > client->req_size) {
            client->error_type = 1;
            client->state = WRITING_RESPONSE;
            fclose(file);
            remove(client->filename);
        }
        else if (bytes_left == 0) {
            client->state = WRITING_RESPONSE;
            fclose(file);
        }
        else {
            client->state = READING_DATA;
            // clear buffer
            // memset(client->buffer, 0, DATA_SIZE);
            client->buffer_size = 0;
        }
        return 0;
    }
    else if (client->state == WRITING_RESPONSE) {
        char* data_to_send;
        size_t size = 0;

        if (client->error_type != -1) {
            // Error response
            const char err_code[] = "ERROR\n";
            if (client->error_type == 0) {
                client->response = malloc(strlen(err_code) + strlen(err_bad_request));
                sprintf(client->response, "%s%s", err_code, err_bad_request);
                sendall(client->fd, client->response, strlen(client->response));
                free(client->response);
            }
            else if (client->error_type == 1) {
                client->response = malloc(strlen(err_code) + strlen(err_bad_file_size));
                sprintf(client->response, "%s%s", err_code, err_bad_file_size);
                sendall(client->fd, client->response, strlen(client->response));
                free(client->response);
            }
            else if (client->error_type == 2) {
                client->response = malloc(strlen(err_code) + strlen(err_no_such_file));
                sprintf(client->response, "%s%s", err_code, err_no_such_file);
                sendall(client->fd, client->response, strlen(client->response));
                free(client->response);
            }
            // sendall(client->fd, "\0", 1);

            client->state = COMPLETE;
            return 0;
        }
        else {
            // Successeful request
            if (client->method == LIST) {
                data_to_send = list_response(client, &size);
                if (size) size--;
                sendall(client->fd, "OK\n", 3);
                sendall(client->fd, &size, sizeof(size_t));
                sendall(client->fd, data_to_send, size);
                free(data_to_send);
            }
            else if (client->method == GET) {
                send_get_response(client);
                return 0;
            }
            else if (client->method == PUT) {
                // Do nothing
                // Write to file was done in READING_DATA
                client->response = malloc(7);
                snprintf(client->response, 7, "OK\n");
                if (sendall(client->fd, client->response, 4) < 0) {
                    perror("sendall");
                    return -1;
                }
                free(client->response);
            }
            else if (client->method == DELETE) {
                // Delete file
                client->response = malloc(7);
                snprintf(client->response, 7, "OK\n");
                if (sendall(client->fd, client->response, 4) < 0) {
                    perror("sendall");
                    return -1;
                }
                free(client->response);
            }
        }
    
        client->state = COMPLETE;
    }
    else if (client->state == COMPLETE) {
        // Do nothing
    }
    return 0;
}

static void handle_client_request(int epollfd, struct Client_info *client) {
    ssize_t count = 0;
    if (client->state == COMPLETE) {
        printf("Client %d request complete\n", client->fd);
        shutdown(client->fd, SHUT_RDWR);
        return;
    }
    if ((client->state == READING_DATA && client->buffer_size == 0) || client->state == READING_VERB) {
        count = recvall(client->fd, client->buffer, DATA_SIZE);
        client->buffer_size = count;
    }
    if (process_request(client) < 0) {
        // error
        cleanup_client(epollfd, client);
        return;
    }
}

static void cleanup_client(int epollfd, struct Client_info *client) {
    // printf("Cleaning up client %d\n", client->fd);    
    epoll_ctl(epollfd, EPOLL_CTL_DEL, client->fd, NULL);
    close(client->fd);
    free(client);
}

static int handle_new_connection(int serverSocket, int epollfd) {
    struct sockaddr in_addr;
    socklen_t in_len = sizeof(in_addr);
    int clientSocket = accept(serverSocket, &in_addr, &in_len);
    if (clientSocket < 0) {
        perror("accept(): ");
        return -1;
    }
    struct Client_info *client = malloc(sizeof(struct Client_info));
    if (client == NULL) {
        perror("malloc(): ");
        return -1;
    }
    // Initialize client
    client->fd = clientSocket;
    client->method = V_UNKNOWN;
    memset(client->filename, 0, MAX_DIRNAME);
    client->fp = NULL;
    client->file_offset = 0;
    client->state = READING_VERB;
    client->error_type = -1;
    client->response = NULL;
    client->buffer_size = 0;
    client->boffset = 0;
    client->req_size = 0;
    struct epoll_event event;
    // event.events = EPOLLIN | EPOLLET;
    event.events = EPOLLIN;
    event.data.ptr = client;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, clientSocket, &event) < 0) {
        perror("epoll_ctl(): ");
        cleanup_client(epollfd, client);
        return -1;
    }
    printf("New connection on fd %d\n", clientSocket);
    return 0;
}

static int create_and_bind(char *port) {
    struct addrinfo *server_info;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port, &hints, &server_info) != 0) {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(errno));
        return -1;
    }
    int sockfd = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(): ");
        return -1;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(): ");
        return -1;
    }
    if (bind(sockfd, server_info->ai_addr, server_info->ai_addrlen) < 0) {
        perror("bind");
        return -1;
    }
    freeaddrinfo(server_info);
    return sockfd;
}