// connection_manager.h
#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <netinet/in.h>
#include "config.h"

void start_server(int server_socket);
void *handle_client_connection(void *new_client_socket);

typedef struct {
    int socket;
    char username[USERNAME_SIZE];
} Client;


#endif // CONNECTION_MANAGER_H
