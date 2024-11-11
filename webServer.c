#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include "http_message.h"

#define DEFAULT_PORT 50001
#define LISTEN_BACKLOG 5
#define BUFFER_SIZE 1024

int respond_to_http_client_message(int sock_fd, http_client_message_t* http_msg, server_info** http_info){

    if(strncmp(http_msg->method, "/static", 7) == 0){

        char* response = "we received a static message\n";
        (*http_info)->bytes_sent = strlen(response);
        write(sock_fd, response, strlen(response));
        (*http_info)->bytes_sent = strlen(response);

    }
    else if(strncmp(http_msg->method, "/stats", 6) == 0){

        char response[MAX_RESPONSE_SIZE];
        snprintf(response, MAX_RESPONSE_SIZE, "Requests: %d\nBytes Recieved: %d\nBytes Sent: %d\n\n", 
                (*http_info)->requests, 
                (*http_info)->bytes_received, 
                (*http_info)->bytes_sent);
        (*http_info)->bytes_sent = strlen(response);
        write(sock_fd, response, strlen(response));
        write(sock_fd, SEPARATOR, SEPARATOR_LENGTH);
        
    }
    else if(strncmp(http_msg->method, "/calc", 5) == 0){

        int a;
        int b;
        char response[MAX_RESPONSE_SIZE];
        sscanf(http_msg->method, "/calc/%d/%d", &a, &b);
        snprintf(response, MAX_RESPONSE_SIZE, "the sum of %d and %d is %d\n\n", a, b, a+b);
        write(sock_fd, response, sizeof(response)); 
        write(sock_fd, SEPARATOR, SEPARATOR_LENGTH);
        (*http_info)->bytes_sent = strlen(response);
        
    }
    else{
        write(sock_fd, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
    }

    return 0;
}

// incoming param was malloced elsewhere; is freed here
void* handleConnection(void* a_client_ptr){

    int a_client = *(int*)a_client_ptr;
    free(a_client_ptr);

    server_info *http_info = malloc(sizeof(server_info));
    http_info->requests = 0;
    http_info->bytes_received = 0;
    http_info->bytes_sent = 0;

    write(a_client, SEPARATOR, SEPARATOR_LENGTH);

    while (1){
        http_client_message_t* http_msg;
        http_read_result_t result;
        read_http_client_message(a_client, &http_msg, &result, &http_info);

        if(result == BAD_REQUEST){

           printf("Bad request\n");
            close(a_client);
            return 0;

        }
        else if(result == CLOSED_CONNECTION){

            printf("Closed connection\n");
            close(a_client);
            return 0;

        }
        else if(result == INVALID_COMMAND){

            write(a_client, ERROR_MESSAGE, strlen(ERROR_MESSAGE));
            write(a_client, SEPARATOR, SEPARATOR_LENGTH);
        }
        else{

            respond_to_http_client_message(a_client, http_msg, &http_info);
            http_client_message_free(http_msg);

        }
    }

    free(http_info);

   return NULL;
}

int main(int argc, char* argv[]){

    int port = DEFAULT_PORT;

    if(argc > 1 && strcmp(argv[1], "-p") == 0){
        port = atoi(argv[2]);
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in socket_address;
    memset(&socket_address, '\0', sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(port);

    int returnval;

    //connect to network port 50001
    returnval = bind(socket_fd, (struct sockaddr*)&socket_address, 
                     sizeof(socket_address));
    if (returnval == -1){
        perror("bind");
        return 1;
    }

    returnval = listen(socket_fd, LISTEN_BACKLOG);

    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    // different client_fd for every "person" you're talking to
    bool read_data = true;

    while(read_data)
    {

        int client_fd = accept(
            socket_fd, (struct sockaddr*)&client_address, &client_address_len);
        if (client_fd == -1){
            perror("accept");
            return 1;
        }

        int* client_fd_ptr = (int*)malloc(sizeof(int));
        *client_fd_ptr = client_fd;

        pthread_t thread;
        pthread_create(&thread, NULL, handleConnection, (void*)client_fd_ptr);

    }

    return 0;

}
