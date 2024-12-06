#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include "connection.h"
#include "commands.h"
#include "../config.h"

// Inicializa as variáveis globais
int current_sequence_number = 0; // Início da sequência
int sock = 0;

int initialize_connection(int is_server)
{
    struct sockaddr_in addr;
    struct sockaddr_ll link_layer_addr; // Usado para raw socket na camada Ethernet
    struct ifreq ifr;                   // Para obter detalhes da interface de rede

    int timeout = TIMEOUT;
    const char *address, *interface;

    if (is_server)
    {
        address = SERVER_ADDRESS;
        interface = SERVER_INTERFACE;
    }
    else
    {
        address = CLIENT_ADDRESS;
        interface = CLIENT_INTERFACE;
    }

    // Criação do socket RAW
    sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0)
    {
        perror("Erro ao criar o socket raw");
        return -1;
    }

    // confifurar timeout aqui
    // ---------

    // Configuração da interface
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0)
    {
        perror("Erro ao obter índice da interface");
        close(sock);
        return -1;
    }

    // Configuração do endereço na camada de enlace (Ethernet)
    memset(&link_layer_addr, 0, sizeof(link_layer_addr));
    link_layer_addr.sll_family = AF_PACKET;
    link_layer_addr.sll_protocol = htons(ETH_P_ALL);
    link_layer_addr.sll_ifindex = ifr.ifr_ifindex;

    // Vincula o socket à interface no modo cliente ou servidor
    if (bind(sock, (struct sockaddr *)&link_layer_addr, sizeof(link_layer_addr)) < 0)
    {
        perror("Erro ao fazer bind no socket raw");
        close(sock);
        return -1;
    }

    if (is_server)
        printf("Servidor iniciado na interface %s\n", interface);
    else
        printf("Cliente configurado para usar a interface %s\n", interface);

    return sock; // Retorna o descritor do socket
}

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
        create_packet(packet, chunk_size, sequence_number++, cmd->type, data);

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
PacketQueue *send_packet_queue(PacketQueue *queue)
{

    PacketNode *p_node = queue->head;
    uint8_t *buffer;

    while (p_node)
    {
        build_packet(buffer, p_node->packet);

        // Envia o buffer de uma só vez
        ssize_t bytes_sent = send(sock, buffer, p_node->packet->tam, 0);
        if (bytes_sent < 0)
        {
            perror("Erro ao enviar buffer");
            return -1;
        }

        // Aguarda a resposta do servidor
        unsigned char buffer[sizeof(Packet)];
        struct sockaddr_ll endereco_servidor;
        socklen_t endereco_len = sizeof(endereco_servidor);

        ssize_t recebido = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&endereco_servidor, &endereco_len);
        if (recebido < 0)
        {
            perror("Erro ao receber resposta do servidor");
            close(sock);
            exit(EXIT_FAILURE);
        }

        Packet *res;

        parse_packet(buffer, res);

        // Verifica o marcador
        if (resposta->marcador != MARCADOR_INICIO)
        {
            printf("Resposta inválida do servidor: marcador incorreto (%x)\n", resposta->marcador);
            close(soquete);
            exit(EXIT_FAILURE);
        }

        // Verifica o CRC
        unsigned char crc_recebido = resposta->crc;
        resposta->crc = 0; // Zera o CRC antes de recalcular
        if (calcula_crc(resposta) != crc_recebido)
        {
            printf("Resposta inválida do servidor: CRC incorreto\n");
            close(soquete);
            exit(EXIT_FAILURE);
        }

        // Verifica o tipo de resposta
        if (resposta->tipo == MESSAGE_ACK)
        {
            printf("Servidor respondeu com ACK!\n");
        }
        else if (resposta->tipo == MESSAGE_NACK)
        {
            printf("Servidor respondeu com NACK!\n");
        }
        else
        {
            printf("Resposta inesperada do servidor. Tipo: %d\n", resposta->tipo);
        }

        p_node = p_node->next;
    }
}

/**
 * @brief Libera a memória de uma fila de pacotes.
 *
 * @param queue Ponteiro para a fila de pacotes a ser liberada.
 * Libera todos os nós e pacotes da fila.
 */
void free_packet_queue(PacketQueue *queue);