#include "connection_manager.h"
#include "message_handler.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

extern Client clients[MAX_CLIENTS];
extern pthread_mutex_t clients_mutex;

// Main function to manage the messages
int process_message(char *buffer, char *sender, int sender_socket) {
    char recipient[USERNAME_SIZE], message[BUFFER_SIZE];

    // /msg command to send private messages
    if (sscanf(buffer, "/msg %s %[^\n]", recipient, message) == 2) {
        char full_message[BUFFER_SIZE];
        sprintf(full_message, "%s (private): %s\n", sender, message);
        send_private_message(full_message, recipient);
    } 
    else if (strcmp(buffer, "/list") == 0) {
        // /list command to list connected clients
        list_connected_clients(sender_socket, sender);
    } else if (strcmp(buffer, "/exit") == 0){
        // /exit command to close the connection
        remove_client(sender_socket);
        return 1; 
    } else {
        // If it is a normal message, broadcast it to all clients
        char full_message[BUFFER_SIZE];
        sprintf(full_message, "%s: %s\n", sender, buffer);
        broadcast_message(full_message, sender_socket);
    }
}

// Listing connected clients to the sender
void list_connected_clients(int client_socket, char *username_now) {
    char client_list[BUFFER_SIZE] = "Connected clients:\n";
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket != 0) {
            if (strcmp(clients[i].username, username_now) == 0){
                continue;
            }  
            strcat(client_list, clients[i].username);
            strcat(client_list, "\n");
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    send(client_socket, client_list, strlen(client_list), 0);
}

// Send a private message to a client
void send_private_message(char *message, char *recipient) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket != 0 && strcmp(clients[i].username, recipient) == 0) {
            send(clients[i].socket, message, strlen(message), 0);
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Broadcast a message to all clients
void broadcast_message(char *message, int exclude_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket != 0 && clients[i].socket != exclude_socket) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Remove client from connected clients
void remove_client(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket == client_socket) {
            clients[i].socket = 0;  // Mark the socket as available
            memset(clients[i].username, 0, sizeof(clients[i].username));  // Clean the username
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
