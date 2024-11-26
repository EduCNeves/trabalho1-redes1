#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "packet.h"

uint8_t calculate_crc(const uint8_t *data, size_t length)
{
    uint8_t crc = 0;
    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i]; // XOR de todos os bytes para cálculo simples de integridade
    }
    return crc;
}

// Implementação do build_packet
void build_packet(uint8_t *buffer, const Packet *packet)
{
    if (!buffer || !packet)
        return;

    // Limpa o buffer antes de preencher
    memset(buffer, 0, PACKET_SIZE);

    // Preenche o marcador de início (byte 0)
    buffer[0] = packet->start_marker;

    // Compacta Tam (6 bits) e Seq (5 bits)
    buffer[1] = (packet->tam & MASK_TAM) << 2;       // Tam ocupa os 6 bits mais altos
    buffer[1] |= (packet->seq >> 3) & MASK_SEQ_HIGH; // 2 bits mais altos de Seq
    buffer[2] = (packet->seq & MASK_SEQ_LOW) << 5;   // 3 bits mais baixos de Seq
    buffer[2] |= (packet->type & MASK_TYPE);         // Tipo ocupa os 5 bits restantes

    // Copia os dados (até o tamanho especificado por Tam)
    memcpy(&buffer[3], packet->data, packet->tam);

    // Calcula o CRC e adiciona no último byte
    buffer[3 + packet->tam] = calculate_crc(buffer, 3 + packet->tam);
}

// Função para decodificar um buffer em um Packet
int parse_packet(const uint8_t *buffer, Packet *packet)
{
    if (!buffer || !packet)
        return -1; // Verifica entrada inválida

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
        return -1; // Dados fora do limite permitido
    }

    // Copia os dados (até o tamanho especificado por Tam)
    memcpy(packet->data, &buffer[3], packet->tam);

    // Calcula e verifica o CRC
    uint8_t calculated_crc = calculate_crc(buffer, 3 + packet->tam);
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

    // Calcula o CRC se não foi fornecido
    uint8_t buffer[PACKET_SIZE + packet->tam];
    build_packet(buffer, packet);
    packet->crc = buffer[3 + packet->tam];
}
