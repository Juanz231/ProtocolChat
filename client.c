#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *receive_messages(void *socket_fd);

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char username[USERNAME_SIZE], buffer[BUFFER_SIZE];
    char ip_address[INET_ADDRSTRLEN], port_str[6]; 
    int port;

    // Ask for the server's IP address
    printf("Enter the server's IP address  (press Enter for 127.0.0.1): ");
    fgets(ip_address, INET_ADDRSTRLEN, stdin);
    ip_address[strcspn(ip_address, "\n")] = '\0';  // Remove newline

    // Ask for the server's port
    printf("Enter the server port (press Enter for 8080): ");
    fgets(port_str, 6, stdin);
    port_str[strcspn(port_str, "\n")] = '\0';  // Remove newline

    // Use default values if none provided    
    if (strlen(ip_address) == 0) {
        strcpy(ip_address, "12// Create the client socket7.0.0.1");
    }
    if (strlen(port_str) == 0) {
        port = 8080;
    } else {
        port = atoi(port_str); // Convert port to an integer
    }

    // Create the client socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("ERR: Error creating socket.");
        exit(EXIT_FAILURE);
    }

    // Set up the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_address, &server_addr.sin_addr) <= 0) {
        perror("ERR: Invalid IP address.");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("ERR: Error connecting to server.");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    // Ask for the username
    printf("Enter your username: ");
    fgets(username, USERNAME_SIZE, stdin);
    username[strcspn(username, "\n")] = '\0';  // Remove newline
    if (write(client_socket, username, strlen(username)) < 0){
      perror("ERR: Error al enviar el nombre de usuario.");
      return 0;
    }
    // Create a thread to receive messages from the server
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, (void*)&client_socket) != 0) {
        perror("ERR: Error creating receiving thread.");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Send messages to the server
    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline

        // If the client types /exit, disconnect from the server
        if (strcmp(buffer, "/exit") == 0) {
            printf("DISC: Disconnecting from server...\n");
            send(client_socket, buffer, BUFFER_SIZE, 0);  // Notify the server
            break;  // Exit the loop to close the connection
        }

        if (send(client_socket, buffer, BUFFER_SIZE, 0) == -1) {
            perror("ERR: Error sending message.");
            break;
        }

    }

    // Wait for the receiving thread to finish
    pthread_join(receive_thread, NULL);

    // Close the client socket
    close(client_socket);
    return 0;
}

// Function to receive messages from the server
void *receive_messages(void *socket_fd) {
    int client_socket = *(int*)socket_fd;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int receive = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (receive > 0) {
            printf("%s", buffer);
        } else if (receive == 0) {
            printf("DISC: Connection closed by server.\n");
            break;
        } else {
            perror("ERR: Error receiving message.");
            break;
        }
    }
    return NULL;
}
