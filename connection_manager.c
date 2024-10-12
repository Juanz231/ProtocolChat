// connection_manager.c
#include "connection_manager.h"
#include "message_handler.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

Client clients[MAX_CLIENTS] = {0};
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Start the server and wait for connections
void start_server(int server_socket) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket;
    pthread_t tid;

    // Accept connections
    while ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len)) >= 0) {
        pthread_mutex_lock(&clients_mutex);
        
        // IMPORTANT: We need to pass a pointer to the client_socket as an argument to the thread function
        int *new_client = malloc(sizeof(int));
        *new_client = client_socket;
        
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clients[i].socket == 0) {
                pthread_create(&tid, NULL, (void*)handle_client_connection, new_client);
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }
}

// Handle the connection of a new client
void *handle_client_connection(void* new_client_socket) {
    
    // VERY IMPORTANT: We need to desferenciate the pointer to the client_socket 
    // and convert to a integer
    int client_socket = *((int *) new_client_socket);
    free(new_client_socket);
    
    char username[USERNAME_SIZE], buffer[BUFFER_SIZE], message[BUFFER_SIZE];
    
    // Receive username from client
    int bytes_received = read(client_socket, username, USERNAME_SIZE);
    if (bytes_received < 0) {
        perror("Error receiving username.");
        return 0;
    }

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket == 0) {
            clients[i].socket = client_socket;
            strcpy(clients[i].username, username);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    sprintf(message, "%s has connected.\n", username);
    printf("%s", message);

    // Receive messages from the client
    while (recv(client_socket, buffer, BUFFER_SIZE, 0) > 0) {
        buffer[strcspn(buffer, "\n")] = '\0';
        
        // Process the message
        int p = process_message(buffer, username, client_socket);
        
        // If the user types /exit, the connection is closed
        if (p == 1) {
            break;
        }

    }

    // Close the connection with the client
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket == client_socket) {
            clients[i].socket = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    sprintf(message, "%s has disconnected.\n", username);
    printf("%s", message);

    close(client_socket);
}
