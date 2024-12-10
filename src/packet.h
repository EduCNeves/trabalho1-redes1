#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <stddef.h>

// Definição de constantes
#define MAX_DATA_SIZE 63   // Tamanho máximo dos dados no pacote (em bytes)
#define PACKET_SIZE_MIN 46 // Tamanho minimo do pacote (em bytes)
#define PACKET_SIZE_MAX 67 // Tamanho minimo do pacote (em bytes)

#define PACKET_START_MARKER 0b01111110

// Definição de mascaras
#define MASK_TAM 0b00111111      // Para isolar os 6 bits mais baixos (Tam)
#define MASK_SEQ_HIGH 0b00000011 // Para os 2 bits mais altos de Seq
#define MASK_SEQ_LOW 0b00000111  // Para os 3 bits mais baixos de Seq
#define MASK_TYPE 0b00011111     // Para isolar os 5 bits mais baixos (Type)

// Estrutura representando um pacote
typedef struct
{
    uint8_t start_marker;        // Marcador de início (8 bits)
    uint8_t tam;                 // Tamanho dos dados (6 bits)
    uint8_t seq;                 // Número de sequência (5 bits)
    uint8_t type;                // Tipo de mensagem (5 bits)
    uint8_t data[MAX_DATA_SIZE]; // Dados (0 a 63 bytes)
    uint8_t crc;                 // CRC (8 bits)
} Packet;

/**
 * @brief Calcula o CRC para um conjunto de dados.
 *
 * @param data Ponteiro para os dados a serem processados.
 * @param length Comprimento dos dados.
 * @return uint8_t Valor do CRC calculado.
 */
uint8_t calculate_crc(const uint8_t *data, size_t length);

/**
 * @brief Monta um pacote em um buffer.
 *
 * @param buffer Buffer onde o pacote será armazenado (deve ter pelo menos PACKET_SIZE bytes).
 * @param packet Estrutura Packet com os dados a serem compactados.
 */
size_t build_packet(uint8_t *buffer, const Packet *packet);

/**
 * @brief Decodifica um buffer em uma estrutura Packet.
 *
 * @param buffer Buffer contendo o pacote compactado.
 * @param packet Ponteiro para a estrutura Packet onde os dados serão armazenados.
 * @return int Retorna 0 em caso de sucesso, ou -1 se o CRC for inválido.
 */
int parse_packet(const uint8_t *buffer, Packet *packet);

/**
 * @brief Cria e inicializa um pacote com os dados fornecidos.
 *
 * @param packet Ponteiro para a estrutura Packet a ser preenchida.
 * @param tam Tamanho dos dados no pacote (máximo 63 bytes).
 * @param seq Número de sequência do pacote.
 * @param type Tipo de mensagem ou comando.
 * @param data Ponteiro para os dados a serem incluídos no pacote.
 * @param crc CRC pré-calculado para o pacote (0 se não calculado ainda).
 *
 * @note O CRC será calculado automaticamente se o valor passado for 0.
 */
void create_packet(Packet *packet, uint8_t tam, uint8_t seq, uint8_t type, const uint8_t *data);

#endif // PACKET_H
