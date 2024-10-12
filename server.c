// server.c
#include "connection_manager.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main() {
    int server_socket;
    struct sockaddr_in server_addr;

    // Se crea el socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0) {
        perror("ERR: Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Se configura la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Se enlaza el socket al puerto
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("ERR: Error in bind");
        exit(EXIT_FAILURE);
    }

    // Se escuchan las conexiones
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("ERR: Error in listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", SERVER_PORT);
    start_server(server_socket);

    close(server_socket);
    return 0;
}
