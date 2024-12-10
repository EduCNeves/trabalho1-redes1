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
 * @return MessageType resposta
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

/**
 * @brief Envia um único pacote e recebe a resposta do servidor.
 *
 * @param sock_fd Descritor do socket.
 * @param packet Ponteiro para o pacote a ser enviado.
 * @param response Ponteiro para o pacote onde a resposta será armazenada.
 * @return int Retorna 0 em caso de sucesso, ou -1 em caso de erro.
 */
int send_packet(const Packet *packet, Packet *response);

/**
 * @brief Recebe pacotes do socket até encontrar a mensagem de fim de transmissão.
 *
 * @return PacketQueue* Ponteiro para a fila de pacotes recebidos. Deve ser liberada após o uso.
 */
PacketQueue *receive_data();

/**
 * @brief Inicializa uma nova fila de pacotes.
 *
 * @return PacketQueue* Ponteiro para a nova fila inicializada ou NULL em caso de falha.
 */
PacketQueue *initialize_packet_queue();

/**
 * @brief Recebe e decodifica um pacote do socket.
 *
 * @param sock Descritor do socket de onde o pacote será recebido.
 * @return Packet* Ponteiro para a estrutura Packet ou NULL em caso de erro.
 *
 * @note O chamador é responsável por liberar a memória alocada para o Packet.
 */
Packet *receive_packet();

/**
 * @brief Adiciona um pacote à fila de pacotes.
 *
 * @param queue Ponteiro para a fila de pacotes.
 * @param packet Ponteiro para o pacote a ser adicionado.
 * @return int Retorna 1 em caso de sucesso ou 0 em caso de falha.
 */
int add_packet_to_queue(PacketQueue *queue, Packet *packet);

/**
 * @brief Envia um pacote de confirmação (ACK) para o cliente.
 *
 * @param sock Descritor do socket para onde o ACK será enviado.
 * @param seq Número de sequência do pacote para o qual o ACK é enviado.
 */
void send_ack(uint8_t seq);

/**
 * @brief Envia um pacote de não confirmação (NACK) para o cliente.
 *
 * @param sock Descritor do socket para onde o NACK será enviado.
 * @param seq Número de sequência do pacote para o qual o NACK é enviado.
 */
void send_nack(uint8_t seq);

/**
 * @brief Envia um pacote de confirmação OK para o socket.
 *
 * @param seq O número de sequência associado à mensagem OK.
 * @return int Retorna 0 em caso de sucesso ou -1 em caso de erro.
 */
int send_ok(uint8_t seq);

/**
 * @brief Incrementa o número de sequência considerando o comportamento cíclico (0 a 31).
 *
 * @return uint8_t O próximo número de sequência (entre 0 e 31).
 */
uint8_t increment_sequence();

#endif
