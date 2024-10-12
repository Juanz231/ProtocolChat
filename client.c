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

    // Requesting IP address and PORT
    printf("Enter the IP address for connect to: (press Enter for 127.0.0.1 or localhost): ");
    fgets(ip_address, INET_ADDRSTRLEN, stdin);
    ip_address[strcspn(ip_address, "\n")] = '\0';  // Remove newline character

    printf("Set the PORT to connect to: (press Enter for 8080): ");
    fgets(port_str, 6, stdin);
    port_str[strcspn(port_str, "\n")] = '\0';  // Remove newline character

    // If user did not enter IP address or PORT, use default
    if (strlen(ip_address) == 0) {
        strcpy(ip_address, "127.0.0.1");
    }
    if (strlen(port_str) == 0) {
        port = 8080;
    } else {
        port = atoi(port_str);  // Convert port to integer
    }

    // Creating a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    
    // Seting up the server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_address, &server_addr.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    // Send username to the server
    printf("Submit your username: ");
    fgets(username, USERNAME_SIZE, stdin);
    username[strcspn(username, "\n")] = '\0';  // Remove newline character
    if (write(client_socket, username, strlen(username)) < 0){
      perror("Error sending username");
      return 0;
    }
    // Create a thread for receiving messages from the server
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, (void*)&client_socket) != 0) {
        perror("Error creating messages receiving thread");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Send messages to the server
    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline character

        // if user types /exit disconnect from server
        if (strcmp(buffer, "/exit") == 0) {
            printf("Desconnecting from the server...\n");
            send(client_socket, buffer, BUFFER_SIZE, 0);  // Notify the sever
            break;  // Exit the loop
        }

        // Real send state.
        if (send(client_socket, buffer, BUFFER_SIZE, 0) == -1) {
            perror("Error sending message");
            break;
        }

    }

    // Wait for the receiving thread to finish
    pthread_join(receive_thread, NULL);

    // Close the socket
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
            printf("Connection terminated from server.\n");
            break;
        } else {
            perror("Error receiving message");
            break;
        }
    }
    return NULL;
}
