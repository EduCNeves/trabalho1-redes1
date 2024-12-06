#ifndef CONFIG_H
#define CONFIG_H

// Configurações gerais do cliente e servidor

// Endereço do servidor
#define SERVER_ADDRESS "127.0.0.1"

// Endereço do cliente (caso necessário para configurações específicas)
#define CLIENT_ADDRESS "127.0.0.1"

// Interface de rede para o servidor (exemplo: eth0)
#define SERVER_INTERFACE "eth0"

// Interface de rede para o cliente (exemplo: wlan0)
#define CLIENT_INTERFACE "wlan0"

// Timeout padrão em segundos para operações de socket
#define SOCKET_TIMEOUT 5

// Tempo limite para operações de socket (em segundos)
// Defina como 0 para desativar o timeout
#define TIMEOUT 5

#endif // CONFIG_H
