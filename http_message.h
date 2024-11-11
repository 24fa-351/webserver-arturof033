#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>

#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#define MIN_BUFFER_LENGTH 10
#define MAX_RESPONSE_SIZE 100
#define SEPARATOR_LENGTH 30
#define ERROR_MESSAGE "\n-------------------\nENTER VALID COMMAND\n-------------------\n\n"
#define SEPARATOR "----------------------------\n\n"

// message from any client
typedef struct msg{
    char* method;
    char* path;
    char* http_version;
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
    MESSAGE, 
    STATIC,
    STATS,
    CALC
} http_read_result_t;

// responses: a message, bad request, closed connection
// allocates and returns a message
void read_http_client_message(int client_sock,
        http_client_message_t** msg,
        http_read_result_t* result,
        server_info** http_info);

void http_client_message_free(http_client_message_t* msg);

#endif
