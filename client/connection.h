#ifndef CONNECTION_H
#define CONNECTION_H

// Declarações das funções para comunicação com o servidor
void connect_to_server();
void send_command(char *command, char *file);
void receive_response();

#endif
