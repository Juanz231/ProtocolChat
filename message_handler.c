#include "connection_manager.h"
#include "message_handler.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

extern Client clients[MAX_CLIENTS];
extern pthread_mutex_t clients_mutex;

// Main function for processing messages from clients
int process_message(char *buffer, char *sender, int sender_socket) {
    char recipient[USERNAME_SIZE], message[BUFFER_SIZE];

    // Private message command /msg
    if (sscanf(buffer, "/msg %s %[^\n]", recipient, message) == 2) {
        char full_message[BUFFER_SIZE];
        snprintf(full_message, sizeof(full_message), "PMSG: %s (private): %s\n", sender, message);
        send_private_message(full_message, recipient);
    }
    // List command to show connected clients
    else if (strcmp(buffer, "/list") == 0) {
        list_connected_clients(sender_socket, sender);
    } 
    // Exit command to disconnect the client
    else if (strcmp(buffer, "/exit") == 0){
        remove_client(sender_socket);
        return 1; 
    } 
    // Broadcast messages to all clients
    else {
        char full_message[BUFFER_SIZE];
        sprintf(full_message, "MSG: %s: %s\n", sender, buffer);
        broadcast_message(full_message, sender_socket);
    }
}

// List all connected clients
void list_connected_clients(int client_socket, char *username_now) {
    char client_list[BUFFER_SIZE] = "UPL: Connected clients:\n";
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

// Send a private message to a specific client
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

// Broadcast a message to all connected clients, excluding the sender
void broadcast_message(char *message, int exclude_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket != 0 && clients[i].socket != exclude_socket) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Remove a client from the clients list when they disconnect
void remove_client(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket == client_socket) {
            clients[i].socket = 0;  // Mark client as disconnected
            memset(clients[i].username, 0, sizeof(clients[i].username));  // Clear the username
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
