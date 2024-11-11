#include "http_message.h"


bool complete_http_message(char* buffer, http_read_result_t* result){

    if(strcmp(buffer, "") != 0 && 
       strlen(buffer) < MIN_BUFFER_LENGTH && 
       strncmp(buffer, "GET ", 4) != 0){

        *result = INVALID_COMMAND;
        return true;

    }
    // if the last four char are "\n"
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
    
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "");
    //printf("Recieved from socket_fd %lu \n", strlen(buffer));

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

        //printf("Recieved from socket_fd %lu and %s \n", strlen(buffer), buffer);
        //write(client_sock, buffer, strlen(buffer));

    }

    if(*result == MESSAGE){

        (*http_info)->requests += 1;
        (*msg)->method = strdup(buffer + 4);

    }

}

void http_client_message_free(http_client_message_t* msg){

    free(msg);

}
