#include <stdlib.h>
#include <string.h>

#include "connection.h"
#include "commands.h"

PacketQueue *convert_command_to_packets(Command *cmd)
{
    if (!cmd)
        return NULL;

    // Cria a fila de pacotes
    PacketQueue *queue = (PacketQueue *)malloc(sizeof(PacketQueue));
    if (!queue)
        return NULL;

    queue->head = NULL;
    queue->tail = NULL;

    // Tamanho dos dados e cálculo do número de pacotes necessários
    size_t total_data_size = 0;
    if (cmd->data_type == STRING)
    {
        total_data_size = strlen(cmd->data.string_data);
    }
    else if (cmd->data_type == FILE_PTR)
    {
        fseek(cmd->data.file_data, 0, SEEK_END);
        total_data_size = ftell(cmd->data.file_data);
        rewind(cmd->data.file_data);
    }

    size_t remaining_data = total_data_size;
    uint8_t seq = 0;

    while (remaining_data > 0)
    {
        // Cria um novo nó para a fila
        PacketNode *node = (PacketNode *)malloc(sizeof(PacketNode));
        if (!node)
        {
            // Libera a fila em caso de erro
            free_packet_queue(queue);
            return NULL;
        }

        // Define o tamanho dos dados do pacote (máximo 63 bytes)
        size_t chunk_size = (remaining_data > MAX_DATA_SIZE) ? MAX_DATA_SIZE : remaining_data;

        uint8_t *data = NULL;

        // Determina a origem dos dados
        if (cmd->data_type == STRING)
        {
            data = (uint8_t *)(cmd->data.string_data + (total_data_size - remaining_data));
        }
        else if (cmd->data_type == FILE_PTR)
        {
            fread(data, 1, chunk_size, cmd->data.file_data);
        }

        // Cria o pacote usando a função
        Packet *packet = &node->packet;
        create_packet(packet, chunk_size, seq++, cmd->type, data);

        // Insere o nó na fila
        node->next = NULL;
        if (!queue->head)
        {
            queue->head = node;
            queue->tail = node;
        }
        else
        {
            queue->tail->next = node;
            queue->tail = node;
        }

        remaining_data -= chunk_size;
    }

    return queue;
}
