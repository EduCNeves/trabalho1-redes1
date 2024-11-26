#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "packet.h"
#include "commands.h"
#include "connection.h"

// Função para imprimir a fila de pacotes
void print_packet_queue(PacketQueue *queue)
{
    PacketNode *current = queue->head;
    while (current)
    {
        Packet *packet = &current->packet;
        printf("Packet [Seq: %d, Type: %d, Tam: %d, CRC: 0x%02X]\n",
               packet->seq, packet->type, packet->tam, packet->crc);
        printf("Data: ");
        for (int i = 0; i < packet->tam; i++)
        {
            printf("%02X", packet->data[i]);
        }
        printf("\n");
        current = current->next;
    }
}

// Função para liberar a memória de uma fila de pacotes
void free_packet_queue(PacketQueue *queue)
{
    if (!queue)
        return;
    PacketNode *current = queue->head;
    while (current)
    {
        PacketNode *temp = current;
        current = current->next;
        free(temp);
    }
    free(queue);
}

int main()
{
    // Teste com STRING
    char test_string[] = "Hello, Packet Testing!";
    Command cmd_string;
    cmd_string.type = COMMAND_BACKUP;
    cmd_string.data_type = STRING;
    cmd_string.data.string_data = test_string;

    // Converte Command para PacketQueue
    PacketQueue *queue_string = convert_command_to_packets(&cmd_string);
    if (!queue_string)
    {
        printf("Erro ao converter STRING Command para PacketQueue.\n");
        return 1;
    }

    printf("Fila de Pacotes (STRING):\n");
    print_packet_queue(queue_string);

    free_packet_queue(queue_string);

    printf("\nTeste concluído.\n");
    return 0;
}
