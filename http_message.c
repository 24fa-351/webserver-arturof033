#include "http_message.h"
#include <stdlib.h>


bool complete_http_message(char* buffer, http_read_result_t* result){

    if(strcmp(buffer, "") != 0 && 
       strlen(buffer) < MIN_BUFFER_LENGTH && 
       strncmp(buffer, "GET ", 4) != 0){

        *result = INVALID_COMMAND;
        return true;

    }
    // if the last four char are "\r\n\r\n"
    if(strcmp(buffer + strlen(buffer) - 4, "\r\n\r\n") == 0){

        *result = MESSAGE;
        return true;

    }
    
    return false;

}

void read_http_client_message(int client_sock,
        http_client_message_t** msg,
        http_read_result_t* result,
        server_info** http_info){

    *msg = malloc(sizeof(http_client_message_t));

    char buffer[MAX_BUFFER_LENGTH];
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "");

    while(!complete_http_message(buffer, result)){

        //adding message to the end of the buffer
        int bytes_read = read(client_sock, buffer + strlen(buffer), 
                              sizeof(buffer) - strlen(buffer) - 1);
        if(bytes_read == 0){
            *result = CLOSED_CONNECTION;
            return;
        }
        if(bytes_read < 0){
            *result = BAD_REQUEST;
            return;
        }

        (*http_info)->bytes_received += bytes_read;

        buffer[strlen(buffer) + bytes_read] = '\0';

    }

    if(*result == MESSAGE){

        (*http_info)->requests += 1;
        (*msg)->method = strdup(buffer - sizeof(buffer) + 4);
        (*msg)->path = strdup(buffer + 4);

    }

}

// response is malloced here; freed elsewhere
char* get_response_to_http_client_message(http_client_message_t* http_msg, server_info* http_info){

    char* response = malloc(sizeof(char*));

    if(strncmp(http_msg->path, "/static", 7) == 0){

        strcpy(response, "we received a static message\n");

    }
    else if(strncmp(http_msg->path, "/stats", 6) == 0){

        snprintf(response, MAX_COMMAND_RESPONSE_SIZE, "Requests: %d\nBytes Recieved: %d\nBytes Sent: %d\n", 
                http_info->requests, 
                http_info->bytes_received, 
                http_info->bytes_sent);
        
    }
    else if(strncmp(http_msg->path, "/calc", 5) == 0){

        int a;
        int b;
        sscanf(http_msg->path, "/calc/%d/%d", &a, &b);
        snprintf(response, MAX_COMMAND_RESPONSE_SIZE, "%d + %d = %d\n", a, b, a+b);
        
    }
    else{

        strcpy(response, ERROR_MESSAGE);

    }

    return response;

}

void fill_http_client_message(http_client_message_t** http_msg, server_info** http_info){

    (*http_msg)->http_version = strdup("HTTP/1.1 200 OK");
    (*http_msg)->host = strdup("127.0.0.1");
    (*http_msg)->body_length = strlen((*http_msg)->body);
    //(*http_msg)->headers = ;

    (*http_info)->bytes_sent += strlen((*http_msg)->http_version)
                              + strlen((*http_msg)->host)
                              + (*http_msg)->body_length
                              + strlen((*http_msg)->body);

}

void print_http_client_message(int client_fd, http_client_message_t* http_msg){

    char* http_client_response = malloc(sizeof(char*));

    snprintf(http_client_response, MAX_HTTP_CLIENT_RESPONSE_SIZE , "HTTP Version: %s\nHost: %s\nBody Length: %d\nBody: \n\n%s\n", 
                http_msg->http_version,
                http_msg->host,
                http_msg->body_length,
                http_msg->body);


    write(client_fd, http_client_response, strlen(http_client_response));

    write(client_fd, SEPARATOR, SEPARATOR_LENGTH);

    free(http_client_response);

    //USE DPRINT

}



void initialize_http_info(server_info** http_info){
    
    (*http_info)->requests = 0;
    (*http_info)->bytes_received = 0;
    (*http_info)->bytes_sent = 0;

}

void http_client_message_free(http_client_message_t* msg){

    free(msg->body);
    free(msg->http_version);
    free(msg->host);
    free(msg);

}
