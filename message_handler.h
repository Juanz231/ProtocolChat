// message_handler.h
#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

int process_message(char *buffer, char *sender, int sender_socket);
void send_private_message(char *message, char *recipient);
void broadcast_message(char *message, int exclude_socket);
void remove_client(int client_socket);
void list_connected_clients(int client_socket, char *username_now);

#endif // MESSAGE_HANDLER_H
