#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>

#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#define MIN_BUFFER_LENGTH 10
#define MAX_BUFFER_LENGTH 1024
#define MAX_COMMAND_RESPONSE_SIZE 100
#define MAX_HTTP_CLIENT_RESPONSE_SIZE 2048
#define SEPARATOR_LENGTH 30
#define ERROR_MESSAGE "\n-------------------\nENTER VALID COMMAND\n-------------------\n\n"
#define SEPARATOR "----------------------------\n\n"

// message from any client
typedef struct msg{
    char* method;
    char* path;
    char* http_version;
    char* host;
    char* body;
    int body_length;
    char* headers;
} http_client_message_t;

typedef struct server_info{
    int requests;
    int bytes_received;
    int bytes_sent;
} server_info;

typedef enum{
    BAD_REQUEST,
    CLOSED_CONNECTION,
    INVALID_COMMAND,
    MESSAGE
} http_read_result_t;

// responses: a message, bad request, closed connection
// allocates and returns a message
void read_http_client_message(int client_sock,
        http_client_message_t** msg,
        http_read_result_t* result,
        server_info** http_info);

char* get_response_to_http_client_message(http_client_message_t* http_msg, server_info* http_info);

void fill_http_client_message(http_client_message_t** http_msg, server_info** http_info);

void print_http_client_message(int client_fd, http_client_message_t*http_msg);

void initialize_http_info(server_info** http_info);

void http_client_message_free(http_client_message_t* msg);

#endif
