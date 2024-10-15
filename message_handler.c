#include "connection_manager.h"
#include "message_handler.h"
#include "config.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

extern Client clients[MAX_CLIENTS];
extern pthread_mutex_t clients_mutex;

// Función principal para el manejo de mensajes
int process_message(char *buffer, char *sender, int sender_socket) {
    char recipient[USERNAME_SIZE], message[BUFFER_SIZE];

    // Comando /msg para mensajes privados
    if (sscanf(buffer, "/msg %s %[^\n]", recipient, message) == 2) {
        char full_message[BUFFER_SIZE];
        snprintf(full_message, sizeof(full_message), "PMSG: %s (private): %s\n", sender, message);
        send_private_message(full_message, recipient);
    } else if (strcmp(buffer, "/list") == 0) {
        list_connected_clients(sender_socket, sender);
    } else if (strcmp(buffer, "/exit") == 0){
        remove_client(sender_socket);
        return 1; 
    } else {
        // Se difunden los mensajes a todos los clientes
        char full_message[BUFFER_SIZE];
        sprintf(full_message, "MSG: %s: %s\n", sender, buffer);
        broadcast_message(full_message, sender_socket);
    }
}

// Se listan los clientes conectados
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

// Se envían los mensajes privados
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

// Se difunden los mensajes a todos los clientes
void broadcast_message(char *message, int exclude_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket != 0 && clients[i].socket != exclude_socket) {
            send(clients[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Se remueve al cliente de la lista
void remove_client(int client_socket) {
    char username[USERNAME_SIZE]={0};
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket == client_socket) {
            strcpy(username, clients[i].username);
            clients[i].socket = 0;  // Marcar el cliente como desconectado
            memset(clients[i].username, 0, sizeof(clients[i].username));  // Limpiar el nombre de usuario
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    char disconnect_message[BUFFER_SIZE];
    snprintf(disconnect_message, sizeof(disconnect_message), "CONN: %s has disconnected.\n", username);

    // Difundir el mensaje a todos los demás clientes
    broadcast_message(disconnect_message, client_socket); 
}
