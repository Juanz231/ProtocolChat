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

    printf("Ingresa la dirección IP del servidor (presiona Enter para 127.0.0.1): ");
    fgets(ip_address, INET_ADDRSTRLEN, stdin);
    ip_address[strcspn(ip_address, "\n")] = '\0';  // Eliminar el salto de línea

    printf("Ingresa el puerto del servidor (presiona Enter para 8080): ");
    fgets(port_str, 6, stdin);
    port_str[strcspn(port_str, "\n")] = '\0';  // Eliminar el salto de línea

    // Si el usuario no ingresa nada, usar valores predeterminados
    if (strlen(ip_address) == 0) {
        strcpy(ip_address, "127.0.0.1");
    }
    if (strlen(port_str) == 0) {
        port = 8080;
    } else {
        port = atoi(port_str);  // Convertir el puerto a número
    }

    // Crear el socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }
    
    // Configurar la dirección del servidor
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_address, &server_addr.sin_addr) <= 0) {
        perror("Dirección IP inválida");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Conectar al servidor
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error al conectar con el servidor");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
    // Pide nombre de Usuario
    printf("Ingresa tu nombre de usuario: ");
    fgets(username, USERNAME_SIZE, stdin);
    username[strcspn(username, "\n")] = '\0';  // Eliminar salto de línea 
    if (write(client_socket, username, strlen(username)) < 0){
      perror("Error al enviar el nombre de usuario");
      return 0;
    }
    // Crear un hilo para recibir mensajes
    pthread_t receive_thread;
    if (pthread_create(&receive_thread, NULL, receive_messages, (void*)&client_socket) != 0) {
        perror("Error al crear el hilo de recepción");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Enviar mensajes al servidor
    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';  // Eliminar el salto de línea

        //Si el cliente escribe /exit se desconecta del servidor
        if (strcmp(buffer, "/exit") == 0) {
            printf("Desconectándote del servidor...\n");
            send(client_socket, buffer, BUFFER_SIZE, 0);  // Notificar al servidor
            break;  // Salir del bucle para cerrar la conexión
        }

        if (send(client_socket, buffer, BUFFER_SIZE, 0) == -1) {
            perror("Error al enviar el mensaje");
            break;
        }

    }

    // Esperar a que el hilo de recepción termine
    pthread_join(receive_thread, NULL);

    // Cerrar el socket
    close(client_socket);
    return 0;
}

// Función para recibir mensajes del servidor
void *receive_messages(void *socket_fd) {
    int client_socket = *(int*)socket_fd;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int receive = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (receive > 0) {
            printf("%s", buffer);
        } else if (receive == 0) {
            printf("Conexión cerrada por el servidor.\n");
            break;
        } else {
            perror("Error al recibir mensaje");
            break;
        }
    }
    return NULL;
}
