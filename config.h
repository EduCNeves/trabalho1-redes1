#ifndef CONFIG_H
#define CONFIG_H

// Configurações gerais do cliente e servidor

// Endereço do servidor
#define SERVER_ADDRESS "127.0.0.1"

// Endereço do cliente (caso necessário para configurações específicas)
#define CLIENT_ADDRESS "127.0.0.1"

// Interface de rede para o servidor (exemplo: eth0)
#define SERVER_INTERFACE "wlp3s0"

// Interface de rede para o cliente (exemplo: wlan0)
#define CLIENT_INTERFACE "wlp3s0"

// Timeout padrão em segundos para operações de socket
#define SOCKET_TIMEOUT 5

// Tempo limite para operações de socket (em segundos)
// Defina como 0 para desativar o timeout
#define TIMEOUT 2

#define MAX_CONSECUTIVE_TIMEOUTS 5

// Define o diretório padrão para operações de backup
#define BACKUP_DIR "backup/"

#define MAX_RETRIES 100

#endif // CONFIG_H
