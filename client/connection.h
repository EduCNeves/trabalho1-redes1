#ifndef CONNECTION_H
#define CONNECTION_H

#include "commands.h" // Para o uso da estrutura Command

// Enum para representar os tipos de mensagem baseados na tabela fornecida
typedef enum {
    // Mensagens de controle
    MESSAGE_ACK = 0b00000,
    MESSAGE_NACK = 0b00001,
    MESSAGE_OK = 0b00010,
    MESSAGE_OK_CHECKSUM = 0b01101,
    MESSAGE_OK_SIZE = 0b01110,
    MESSAGE_SIZE = 0b01111,
    MESSAGE_DATA = 0b10000,
    MESSAGE_END_DATA = 0b10001,
    MESSAGE_ERROR = 0b11111,
    
    // Códigos de comando
    COMMAND_BACKUP = 0b10100,
    COMMAND_RESTORE = 0b10101,
    COMMAND_CHECK = 0b10110
} MessageType;

// Estrutura de pacote individual
typedef struct {
    char data[64];   // Dados contidos no pacote
    int length;      // Comprimento dos dados no pacote
    MessageType type; // Tipo da mensagem, conforme a tabela
} Packet;

// Estrutura para um nó de pacote na fila
typedef struct PacketNode {
    Packet packet;
    struct PacketNode *next;
} PacketNode;

// Estrutura para a fila de pacotes
typedef struct {
    PacketNode *head;
    PacketNode *tail;
} PacketQueue;

/**
 * @brief Converte um comando em uma fila de pacotes para envio.
 * 
 * @param cmd Ponteiro para a estrutura Command contendo os dados do comando.
 * @return PacketQueue* Ponteiro para a fila de pacotes resultante. 
 * O chamador é responsável por liberar a fila após o uso.
 */
PacketQueue *convert_command_to_packets(Command *cmd);

/**
 * @brief Converte uma fila de pacotes em uma estrutura Command.
 * 
 * @param queue Ponteiro para a fila de pacotes a ser convertida.
 * @return Command Estrutura Command preenchida com os dados da fila de pacotes.
 * 
 * @note A fila de pacotes deve ser liberada após a conversão.
 */
Command convert_packets_to_command(PacketQueue *queue);

/**
 * @brief Envia uma fila de pacotes através da conexão e recebe a resposta.
 * 
 * @param queue Ponteiro para a fila de pacotes a ser enviada.
 * @return PacketQueue* Ponteiro para a fila de pacotes de resposta recebida.
 * A fila recebida deve ser liberada após o uso.
 */
PacketQueue *send_packet_queue(PacketQueue *queue);

/**
 * @brief Libera a memória de uma fila de pacotes.
 * 
 * @param queue Ponteiro para a fila de pacotes a ser liberada.
 * Libera todos os nós e pacotes da fila.
 */
void free_packet_queue(PacketQueue *queue);

#endif
