#ifndef CONNECTION_H
#define CONNECTION_H

#include "packet.h" // Para Packet e PacketQueu
#include "commands.h"

// Estrutura para um nó de pacote na fila
typedef struct PacketNode
{
    Packet *packet;
    struct PacketNode *next;
} PacketNode;

// Estrutura para a fila de pacotes
typedef struct
{
    PacketNode *head;
    PacketNode *tail;
} PacketQueue;

// Variáveis globais para controle de conexão
extern int sequence_number; // Número inicial de sequência
extern int sock;

/**
 * @brief Inicializa a conexão, incluindo o socket e variáveis de controle globais.
 *
 * @param is_server Indica se o socket será usado como servidor (1 para servidor, 0 para cliente).
 * @return int Descritor de socket (positivo em caso de sucesso) ou -1 em caso de erro.
 *
 * @note A função utiliza variáveis globais previamente definidas para controlar a conexão:
 *       - `port` (porta do socket).
 *       - `address` (endereço IP para cliente).
 *       - `timeout` (tempo limite em segundos).
 *       - `current_sequence_number` (inicializado como 0).
 */
int initialize_connection(int is_server);

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
