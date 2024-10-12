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

// Se inicia el servidor
void start_server(int server_socket) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_socket;
    pthread_t tid;
    printf("CONN: Server started\n");
    while ((client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len)) >= 0) {
        pthread_mutex_lock(&clients_mutex);
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
    printf("The different threads were created for each client.\n");
}

// Se maneja la conexión del cliente
void *handle_client_connection(void* new_client_socket) {
    int client_socket = *((int *) new_client_socket);
    free(new_client_socket);
    char username[USERNAME_SIZE], buffer[BUFFER_SIZE], message[BUFFER_SIZE];
    
    // char * welcome_message = "Bienvenido al chat, ingresa tu nombre de usuario: ";
    // int wrr;
    
    // if ((wrr = write(client_socket, welcome_message, strlen(welcome_message))) < 0){
    //    perror("Error al enviar la solicitud");
    //    return 0;
    // }

    // Se recibe el nombre de usuario
    int bytes_received = read(client_socket, username, USERNAME_SIZE);
    if (bytes_received < 0) {
        perror("ERR: Error receiving username.");
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

    snprintf(message, sizeof(message),"CONN: %s has been connected\n", username);
    printf("%s", message);

    // Se reciben los mensajes del cliente
    while (recv(client_socket, buffer, BUFFER_SIZE, 0) > 0) {
        buffer[strcspn(buffer, "\n")] = '\0';

        int p = process_message(buffer, username, client_socket);
        if (p == 1) {
            break;
        }

    }

    // Se desconecta el cliente
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].socket == client_socket) {
            clients[i].socket = 0;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    snprintf(message, sizeof(message),"DISC: %s has been disconnected\n", username);
    printf("%s", message);
    close(client_socket);
}