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
#include <sys/time.h>

#include "connection.h"
#include "commands.h"
#include "util.h"
#include "../config.h"

// Inicializa as variáveis globais
int sequence_number = 0; // Início da sequência
int ntimeout = 0;        // Início da sequência
int sock = 0;

int initialize_connection(int is_server)
{
    // struct sockaddr_in addr;
    struct sockaddr_ll link_layer_addr; // Usado para raw socket na camada Ethernet
    struct ifreq ifr;                   // Para obter detalhes da interface de rede

    // int timeout = TIMEOUT;
    const char *interface;

    if (is_server)
    {
        // address = SERVER_ADDRESS;
        interface = SERVER_INTERFACE;
    }
    else
    {
        // address = CLIENT_ADDRESS;
        interface = CLIENT_INTERFACE;
    }

    // Criação do socket RAW
    sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0)
    {
        perror("Erro ao criar o socket raw");
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        perror("Erro ao configurar timeout no socket");
        close(sock);
        return -1;
    }

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
    PacketQueue *queue = initialize_packet_queue();
    if (!queue)
        return NULL;

    // Tamanho dos dados e cálculo do número de pacotes necessários
    size_t total_data_size = 0;

    // Buffer local para armazenar os dados
    uint8_t data[MAX_DATA_SIZE];
    memset(data, 0, MAX_DATA_SIZE);

    if (cmd->data_type == COMMAND)
    {
        if (cmd->data.string_data)
        {
            total_data_size = strlen(cmd->data.string_data);
        }
        else
        {
            // Cria o pacote usando a função
            Packet *packet = (Packet *)malloc(sizeof(Packet));
            if (!packet)
            {
                // Libera a fila em caso de erro
                free_packet_queue(queue);
                return NULL;
            }

            create_packet(packet, 0, increment_sequence(), cmd->type, data);

            add_packet_to_queue(queue, packet);

            return queue;
        }
    }
    else if (cmd->data_type == SIZE)
    {
        total_data_size = sizeof(long);
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
        // Cria o pacote usando a função
        Packet *packet = (Packet *)malloc(sizeof(Packet));
        if (!packet)
        {
            // Libera a fila em caso de erro
            free_packet_queue(queue);
            return NULL;
        }

        // Define o tamanho dos dados do pacote (máximo 63 bytes)
        size_t chunk_size = (remaining_data > MAX_DATA_SIZE) ? MAX_DATA_SIZE : remaining_data;

        // Determina os dados a serem copiados
        if (cmd->data_type == COMMAND)
        {
            memcpy(data, cmd->data.string_data + (total_data_size - remaining_data), chunk_size);
        }
        else if (cmd->data_type == FILE_PTR)
        {
            fread(data, 1, chunk_size, cmd->data.file_data);
        }
        else if (cmd->data_type == SIZE)
        {
            // Copia o valor long para o buffer local de dados
            memcpy(data, &cmd->data.long_data, sizeof(long));
            chunk_size = sizeof(long); // Ajusta o tamanho do chunk para o tamanho de long
        }

        // Cria o pacote com os dados determinados
        create_packet(packet, chunk_size, increment_sequence(), cmd->type, data);

        add_packet_to_queue(queue, packet);

        remaining_data -= chunk_size;
    }

    if (cmd->type == MESSAGE_DATA || cmd->type == MESSAGE_LIST_FILES)
    {
        Packet *end_packet = malloc(sizeof(Packet));
        if (!end_packet)
        {
            free_packet_queue(queue);
            return NULL;
        }

        create_packet(end_packet, 0, increment_sequence(), MESSAGE_END_DATA, NULL);

        add_packet_to_queue(queue, end_packet);
    }

    return queue;
}

Command convert_packets_to_command(PacketQueue *queue)
{
    Command cmd;
    memset(&cmd, 0, sizeof(Command)); // Inicializa o comando com valores padrão

    if (!queue || !queue->head)
    {
        fprintf(stderr, "Erro: Fila de pacotes vazia ou inválida.\n");
        cmd.type = MESSAGE_ERROR; // Define como erro
        return cmd;
    }

    // Obtém o tipo do comando a partir do primeiro pacote
    cmd.type = queue->head->packet->type;

    // Reconstrói os dados do comando com base no tipo
    if (cmd.type == MESSAGE_DATA || cmd.type == MESSAGE_END_DATA)
    {
        // Tipo de dado FILE_PTR
        cmd.data_type = FILE_PTR;

        // Cria um arquivo temporário para armazenar os dados recebidos
        cmd.data.file_data = tmpfile();
        if (!cmd.data.file_data)
        {
            fprintf(stderr, "Erro ao criar arquivo temporário.\n");
            cmd.type = MESSAGE_ERROR;
            return cmd;
        }

        // Escreve os dados dos pacotes no arquivo temporário
        PacketNode *current = queue->head;
        while (current)
        {
            fwrite(current->packet->data, 1, current->packet->tam, cmd.data.file_data);
            current = current->next;
        }
        rewind(cmd.data.file_data); // Retorna ao início do arquivo
    }
    else if (cmd.type == MESSAGE_SIZE)
    {
        cmd.data_type = SIZE;

        long n;
        memset(&n, 0, sizeof(long)); // Define valor padrão para evitar lixo
        // Copia os dados da fila para a string
        memcpy(&n, queue->head->packet->data, queue->head->packet->tam);
        cmd.data.long_data = n;
    }
    else
    {
        // Tipo de dado STRING
        cmd.data_type = COMMAND;

        // Calcula o tamanho total dos dados
        size_t total_size = 0;
        PacketNode *current = queue->head;
        while (current)
        {
            total_size += current->packet->tam;
            current = current->next;
        }

        // Aloca memória para a string
        cmd.data.string_data = malloc(total_size + 1);
        if (!cmd.data.string_data)
        {
            fprintf(stderr, "Erro ao alocar memória para string.\n");
            cmd.type = MESSAGE_ERROR;
            return cmd;
        }

        // Copia os dados da fila para a string
        current = queue->head;
        char *ptr = cmd.data.string_data;
        while (current)
        {
            memcpy(ptr, current->packet->data, current->packet->tam);
            ptr += current->packet->tam;
            current = current->next;
        }
        *ptr = '\0'; // Finaliza a string com o caractere nulo
    }

    return cmd;
}

int send_packet(const Packet *packet, Packet *response)
{
    if (!packet || !response)
    {
        return -1;
    }

    // Constrói o buffer do pacote
    uint8_t buffer[PACKET_SIZE_MAX];
    size_t packet_size = build_packet(buffer, packet);

    // Envia o pacote
    ssize_t bytes_sent = send(sock, buffer, packet_size + 1, 0);
    if (bytes_sent < 0)
    {
        fprintf(stderr, "bytes_sent\n");
        return -1;
    }

    fprintf(stderr, "Comando enviado: %u, tipo: %u\n", packet->seq, packet->type);

    // Recebe o pacote de resposta
    errno = 0;
    Packet *received_packet = receive_packet();

    while (running)
    {
        if (!received_packet)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                printf("Erro de timeout\n");
                if (ntimeout++ > MAX_CONSECUTIVE_TIMEOUTS)
                {
                    fprintf(stderr, "Erro de timeout (%ds %dx). packet: %u, type: %u\nPrograma abortado!\n", TIMEOUT, MAX_CONSECUTIVE_TIMEOUTS, packet->seq, packet->type);
                    running = 0;
                    return -2; // Código específico para timeout
                }
                return -1;
            }
        }
        else if (received_packet->seq == packet->seq)
        {
            break;
        }
        errno = 0;
        received_packet = receive_packet();
    }

    ntimeout = 0;

    // Se a resposta não for um ACK, envia um NACK e retorna erro
    if (received_packet->type == MESSAGE_NACK)
    {
        fprintf(stderr, "Recebido %u para o pacote.\n", received_packet->type);
        free(received_packet); // Libera o pacote temporário
        return -1;
    }

    // Copia o pacote de resposta para o parâmetro fornecido
    memcpy(response, received_packet, sizeof(Packet));
    free(received_packet); // Libera o pacote temporário
    return 0;              // Sucesso
}

PacketQueue *send_packet_queue(PacketQueue *queue)
{
    if (!queue || !queue->head)
    {
        fprintf(stderr, "Erro: Fila de pacotes inválida ou vazia.\n");
        return NULL;
    }

    PacketQueue *response_queue = initialize_packet_queue();
    if (!response_queue)
    {
        fprintf(stderr, "Erro ao inicializar a fila de respostas.\n");
        return NULL;
    }

    PacketNode *current_node = queue->head;

    while (current_node)
    {
        Packet *response = malloc(sizeof(Packet));
        if (!response)
        {
            perror("Erro ao alocar memória para pacote de resposta");
            free_packet_queue(response_queue);
            return NULL;
        }

        int attempts = 0;
        int result = -1;

        // Envia o pacote e tenta novamente em caso de falha
        while (attempts < MAX_RETRIES)
        {
            result = send_packet(current_node->packet, response);
            if (result >= 0)
                break;
            if (result == -2)
                return NULL;

            attempts++;
        }

        if (result < 0)
        {
            fprintf(stderr, "Erro: Número máximo de tentativas atingido para o pacote (seq: %d).\n",
                    current_node->packet->seq);
            free(response);
            free_packet_queue(response_queue);
            return NULL;
        }

        // Adiciona a resposta válida à fila de respostas
        if (!add_packet_to_queue(response_queue, response))
        {
            fprintf(stderr, "Erro ao adicionar resposta à fila de respostas.\n");
            free(response);
            free_packet_queue(response_queue);
            return NULL;
        }

        current_node = current_node->next;
    }

    return response_queue;
}

void free_packet_queue(PacketQueue *queue)
{
    if (!queue)
    {
        return; // Se a fila for nula, nada a liberar
    }

    PacketNode *current = queue->head;
    PacketNode *next;
    while (current)
    {
        next = current->next; // Armazena o próximo nó
        if (current->packet)
        {
            free(current->packet); // Libera o pacote associado ao nó
        }
        free(current);  // Libera o nó atual
        current = next; // Avança para o próximo nó
    }

    // Reseta os ponteiros da fila para garantir consistência
    queue->head = NULL;
    queue->tail = NULL;
}

PacketQueue *initialize_packet_queue()
{
    PacketQueue *queue = malloc(sizeof(PacketQueue));
    if (!queue)
    {
        return NULL;
    }

    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

Packet *receive_packet()
{
    uint8_t buffer[PACKET_SIZE_MAX];
    memset(buffer, 0, PACKET_SIZE_MAX);

    ssize_t bytes_received = recv(sock, buffer, PACKET_SIZE_MAX, 0);
    if (bytes_received < 0)
    {
        return NULL;
    }

    if (buffer[0] != PACKET_START_MARKER)
    {
        return NULL;
    }

    Packet *packet = malloc(sizeof(Packet));
    if (!packet)
    {
        return NULL;
    }

    if (parse_packet(buffer, packet) < 0)
    {
        free(packet);
        return NULL;
    }

    return packet;
}

int add_packet_to_queue(PacketQueue *queue, Packet *packet)
{
    if (!queue || !packet)
        return 0;

    PacketNode *node = malloc(sizeof(PacketNode));
    if (!node)
    {
        return 0;
    }

    node->packet = packet;
    node->next = NULL;

    if (!queue->head)
    {
        queue->head = node;
        queue->tail = node;
    }
    else
    {
        if (queue->tail->packet->seq < packet->seq)
        {
            queue->tail->next = node;
            queue->tail = node;
        }
        else
        {
            free(node);
            return -1;
        }
    }

    return 1;
}

void send_ack(uint8_t seq)
{
    Packet ack_packet = {0};
    ack_packet.start_marker = PACKET_START_MARKER;
    ack_packet.type = MESSAGE_ACK;
    ack_packet.seq = seq;
    ack_packet.tam = 0;

    uint8_t ack_buffer[PACKET_SIZE_MAX];
    build_packet(ack_buffer, &ack_packet);

    if (send(sock, ack_buffer, PACKET_SIZE_MAX, 0) < 0)
    {
        perror("Erro ao enviar ACK");
    }
}

void send_nack(uint8_t seq)
{
    Packet nack_packet = {0};
    nack_packet.start_marker = PACKET_START_MARKER;
    nack_packet.type = MESSAGE_NACK;
    nack_packet.seq = seq;
    nack_packet.tam = 0;

    uint8_t nack_buffer[PACKET_SIZE_MAX];
    build_packet(nack_buffer, &nack_packet);

    if (send(sock, nack_buffer, PACKET_SIZE_MAX, 0) < 0)
    {
        perror("Erro ao enviar NACK");
    }
}

PacketQueue *receive_data()
{
    PacketQueue *queue = initialize_packet_queue();
    if (!queue)
    {
        fprintf(stderr, "Erro ao inicializar a fila de pacotes.\n");
        return NULL;
    }

    while (running)
    {
        Packet *packet = receive_packet();
        if (!packet)
        {
            continue; // Ignora erros e tenta novamente
        }

        if (packet->type == MESSAGE_END_DATA)
        {
            send_ack(packet->seq);
            free(packet);
            break; // Fim da transmissão
        }

        if (packet->type != MESSAGE_DATA && packet->type != MESSAGE_LIST_FILES)
        {
            send_nack(packet->seq + 1);
            free(packet);
            continue; // Ignora pacotes inválidos
        }

        if (!add_packet_to_queue(queue, packet))
        {
            fprintf(stderr, "Erro ao adicionar pacote à fila.\n");
            free(packet);
            free_packet_queue(queue);
            return NULL;
        }

        send_ack(packet->seq);
    }

    return queue;
}

int send_ok(uint8_t seq)
{
    if (sock < 0)
    {
        fprintf(stderr, "Erro: Socket inválido.\n");
        return -1;
    }

    // Cria o pacote OK
    Packet ok_packet = {0};
    ok_packet.start_marker = PACKET_START_MARKER;
    ok_packet.type = MESSAGE_OK;
    ok_packet.seq = seq;
    ok_packet.tam = 0;

    // Constrói o buffer para envio
    uint8_t buffer[PACKET_SIZE_MAX];
    size_t packet_size = build_packet(buffer, &ok_packet);

    // Envia o pacote
    ssize_t bytes_sent = send(sock, buffer, packet_size, 0);
    if (bytes_sent < 0)
    {
        perror("Erro ao enviar pacote OK");
        return -1;
    }
    return 0;
}

uint8_t increment_sequence()
{
    return (sequence_number++) & 0x1F; // Incrementa e aplica máscara para manter 5 bits
}