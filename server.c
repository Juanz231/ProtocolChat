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

    // Creating the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == 0) {
        perror("Error creating the socket");
        exit(EXIT_FAILURE);
    }

    // Setting up the server (Type, IP and PORT)
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Binding the socket to the IP and PORT
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error at binding");
        exit(EXIT_FAILURE);
    }

    // Listening to connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Error at listening");
        exit(EXIT_FAILURE);
    }

    printf("Server listening at port %d...\n", SERVER_PORT);
    start_server(server_socket);

    close(server_socket);
    return 0;
}
