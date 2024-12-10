#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "packet.h"

uint8_t calculate_crc(const uint8_t *data, size_t length)
{
    if (!data || length == 0)
    {
        fprintf(stderr, "Erro: Dados inválidos ou comprimento zero no cálculo do CRC.\n");
        return 0;
    }

    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i]; // XOR byte a byte
    }
    return crc;
}

size_t build_packet(uint8_t *buffer, const Packet *packet)
{
    if (!buffer || !packet)
        return 0;

    // Limpa o buffer antes de preencher
    memset(buffer, 0, PACKET_SIZE_MAX);

    // Preenche o marcador de início (byte 0)
    buffer[0] = packet->start_marker;

    // Calcula o tamanho real do pacote
    size_t original_size = 3 + packet->tam + 1; // Cabeçalho (3) + dados (tam) + CRC (1)
    size_t packet_size = original_size;
    size_t tam = packet->tam;
    if (packet_size < PACKET_SIZE_MIN)
    {
        packet_size = PACKET_SIZE_MIN;
        tam = -3 + packet_size - 1;
    }

    // Compacta Tam (6 bits) e Seq (5 bits)
    buffer[1] = (tam & MASK_TAM) << 2;               // Tam ocupa os 6 bits mais altos
    buffer[1] |= (packet->seq >> 3) & MASK_SEQ_HIGH; // 2 bits mais altos de Seq
    buffer[2] = (packet->seq & MASK_SEQ_LOW) << 5;   // 3 bits mais baixos de Seq
    buffer[2] |= (packet->type & MASK_TYPE);         // Tipo ocupa os 5 bits restantes

    // Copia os dados (até o tamanho especificado por Tam)
    memcpy(&buffer[3], packet->data, packet->tam);

    // Garantir que o pacote atenda ao tamanho mínimo
    if (original_size < PACKET_SIZE_MIN)
    {
        memset(&buffer[original_size], 0, PACKET_SIZE_MIN - original_size); // Preenchimento
    }

    // Calcula o CRC apenas nos campos Tam, Seq, Type e Dados
    size_t crc_start = 1;                // Começa a partir do campo `tam` no buffer
    size_t crc_length = 2 + packet->tam; // Inclui `tam`, `seq`, `type` e os dados
    buffer[3 + tam] = calculate_crc(&buffer[crc_start], crc_length);

    return packet_size;
}

// Função para decodificar um buffer em um Packet
int parse_packet(const uint8_t *buffer, Packet *packet)
{
    if (!buffer || !packet)
    {
        printf("Verifica entrada inválida\n");
        return -1; // Verifica entrada inválida
    }

    // Inicializa a estrutura Packet
    memset(packet, 0, sizeof(Packet));

    // Extrai o marcador de início (byte 0)
    packet->start_marker = buffer[0];

    // Extrai Tam (6 bits) e Seq (5 bits)
    packet->tam = (buffer[1] >> 2) & MASK_TAM;         // 6 bits mais altos 0b00111111
    packet->seq = ((buffer[1] & MASK_SEQ_HIGH) << 3) | // 2 bits mais baixos 0b00000011
                  ((buffer[2] >> 5) & MASK_SEQ_LOW);   // 3 bits mais altos 0b00000111

    // Extrai o tipo (5 bits mais baixos de buffer[2])
    packet->type = buffer[2] & MASK_TYPE;

    // Verifica se o tamanho dos dados é válido
    if (packet->tam > MAX_DATA_SIZE)
    {
        printf("Dados fora do limite permitido\n");
        return -1; // Dados fora do limite permitido
    }

    // Copia os dados (até o tamanho especificado por Tam)
    memcpy(packet->data, &buffer[3], packet->tam);

    // Calcula e verifica o CRC
    if (packet->tam == 0)
    {
        printf("CRC inválidoo\n");
        return -1; // CRC inválido
    }

    size_t crc_start = 1;                // Campo `tam` no buffer
    size_t crc_length = 2 + packet->tam; // Campos compactados e dados
    uint8_t calculated_crc = calculate_crc(&buffer[crc_start], crc_length);

    if (calculated_crc != buffer[3 + packet->tam])
    {
        return -1; // CRC inválido
    }

    // Armazena o CRC no Packet (opcional, para depuração)
    packet->crc = calculated_crc;

    return 0; // Sucesso
}

void create_packet(Packet *packet, uint8_t tam, uint8_t seq, uint8_t type, const uint8_t *data)
{
    if (!packet)
        return; // Verifica se o ponteiro é válido

    // Define o marcador de início
    packet->start_marker = PACKET_START_MARKER;

    // Valida e define o tamanho dos dados (máximo permitido)
    packet->tam = (tam > MAX_DATA_SIZE) ? MAX_DATA_SIZE : tam;

    // Define o número de sequência e tipo
    packet->seq = seq;
    packet->type = type;

    // Copia os dados para o pacote
    if (data && packet->tam > 0)
    {
        memcpy(packet->data, data, packet->tam);
    }
    else
    {
        memset(packet->data, 0, MAX_DATA_SIZE); // Zera os dados se não houver entrada
    }
}
