#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>

#include "../src/connection.h"
#include "../src/packet.h"
#include "../src/commands.h"
#include "../src/util.h"

int main()
{
    // Configura o manipulador de sinal para encerrar o servidor
    signal(SIGINT, handle_signal);

    // Inicializa a conexão como servidor
    int sock_fd = initialize_connection(1); // 1 indica que é servidor
    if (sock_fd < 0)
    {
        fprintf(stderr, "Erro ao inicializar o servidor.\n");
        return EXIT_FAILURE;
    }

    printf("Servidor iniciado e aguardando comandos...\n");

    while (running)
    {
        uint8_t recv_buffer[PACKET_SIZE_MAX];
        uint8_t send_buffer[PACKET_SIZE_MAX];
        Packet request, response;

        // Aguarda um buffer do cliente
        ssize_t bytes_received = recv(sock_fd, recv_buffer, sizeof(recv_buffer), 0);
        if (bytes_received < 0)
        {
            perror("Erro ao receber pacote");
            continue;
        }

        if (recv_buffer[0] != PACKET_START_MARKER)
        {
            continue;
        }

        // Converte o buffer recebido em um Packet
        if (parse_packet(recv_buffer, &request) < 0)
        {
            fprintf(stderr, "Erro ao decodificar o pacote recebido\n");
            continue;
        }

        // Processa o pacote recebido
        printf("Comando recebido: Tipo %u, Sequência %u\n", request.type, request.seq);

        // Processa o pacote recebido
        memset(&response, 0, sizeof(Packet)); // Inicializa o pacote de resposta
        response.start_marker = PACKET_START_MARKER;
        response.seq = request.seq; // Mantém o número de sequência do pedido
        response.tam = 0;           // Sem dados na resposta

        ssize_t bytes_sent;
        size_t packet_size;
        Command cmd;

        // Manipula o comando com base no tipo
        switch (request.type)
        {
        case COMMAND_LIST_FILES:
            // Exemplo: Lista os arquivos no servidor

            while (send_ok(request.seq))
                ;

            printf("Resposta: Tipo %u, Sequência %u\n", MESSAGE_OK, request.seq);

            // Prepara o comando de teste
            memset(&cmd, 0, sizeof(Command));
            cmd.type = MESSAGE_LIST_FILES;
            cmd.data_type = COMMAND;

            char *files = list_files();
            if (files)
            {
                cmd.data.string_data = malloc(strlen(files) + 1); // Aloca memória para a string
                if (!cmd.data.string_data)
                {
                    fprintf(stderr, "Erro: Falha ao alocar memória para cmd.data.string_data\n");
                    return COMMAND_MEMORY_ERROR; // Trate o erro adequadamente
                }

                snprintf(cmd.data.string_data, strlen(files) + 1, "%s", files);

                // Converte o comando para uma fila de pacotes
                PacketQueue *queue = convert_command_to_packets(&cmd);
                if (!queue)
                {
                    free(cmd.data.string_data);
                    return COMMAND_MEMORY_ERROR;
                }

                // Envia a fila de pacotes e recebe a resposta
                send_packet_queue(queue);

                printf("Resposta: Tipo %u, Sequência %u - %u, StrLen: %ld\n", MESSAGE_LIST_FILES, queue->head->packet->seq, queue->tail->packet->seq, strlen(files));

                free_packet_queue(queue); // Libera a fila enviada
                free(files);
                continue;
            }

            response.type = MESSAGE_ERROR;

            // Converte o Packet de resposta em um buffer
            packet_size = build_packet(send_buffer, &response);

            // Envia o buffer de resposta ao cliente
            bytes_sent = send(sock_fd, send_buffer, packet_size, 0);
            if (bytes_sent < 0)
            {
                perror("Erro ao enviar resposta");
                continue;
            }
            break;

        case COMMAND_BACKUP:
            // TODO: Implementar lógica para receber e armazenar arquivo
            printf("Comando de backup recebido.\n");

            PacketQueue *queue = initialize_packet_queue();
            if (!queue)
            {
                fprintf(stderr, "Erro ao inicializar a fila de respostas.\n");
                continue;
            }

            add_packet_to_queue(queue, &request);
            cmd = convert_packets_to_command(queue);
            const char *filename = cmd.data.string_data;
            printf("Filename: %s\n", cmd.data.string_data);

            FILE *f = create_file(filename);
            if (!f)
            {
                while (sendError(request.seq, "não deu pra criar"))
                    ;
                continue;
            }

            while (send_ok(request.seq))
                ;

            Packet *r = receive_packet();
            int attempts = 0;
            while (attempts++ < MAX_RETRIES)
            {
                if (r)
                {
                    if (r->type == MESSAGE_SIZE)
                    {
                        break;
                    }
                }
                r = receive_packet();
            }

            PacketQueue *queue_size = initialize_packet_queue();
            if (!queue_size)
            {
                fprintf(stderr, "Erro ao inicializar a fila de respostas.\n");
                continue;
            }

            add_packet_to_queue(queue_size, r);
            cmd = convert_packets_to_command(queue_size);

            if (!has_sufficient_space(cmd.data.string_data, cmd.data.long_data))
            {
                while (sendError(r->seq, "Não tem espaço"))
                    ;
                continue;
            }

            printf("Mensagem de tamanho recebido. %ld\n", cmd.data.long_data);

            while (send_ok(r->seq))
                ;

            PacketQueue *queue_file = initialize_packet_queue();
            if (!queue_size)
            {
                fprintf(stderr, "Erro ao inicializar a fila de respostas.\n");
                continue;
            }

            queue_file = receive_data();
            cmd = convert_packets_to_command(queue_file);

            printf("Arquivo recebido. type: %d\n", cmd.type);

            break;

        case COMMAND_RESTORE:
            // TODO: Implementar lógica para enviar arquivo ao cliente
            printf("Comando de restauração recebido.\n");
            response.type = MESSAGE_ACK;
            // Converte o Packet de resposta em um buffer
            packet_size = build_packet(send_buffer, &response);

            // Envia o buffer de resposta ao cliente
            bytes_sent = send(sock_fd, send_buffer, packet_size, 0);
            if (bytes_sent < 0)
            {
                perror("Erro ao enviar resposta");
                continue;
            }
            break;

        case COMMAND_CHECK:
            // TODO: Implementar lógica para verificar integridade de arquivos
            printf("Comando de verificação recebido.\n");
            response.type = MESSAGE_ACK;
            // Converte o Packet de resposta em um buffer
            packet_size = build_packet(send_buffer, &response);

            // Envia o buffer de resposta ao cliente
            bytes_sent = send(sock_fd, send_buffer, packet_size, 0);
            if (bytes_sent < 0)
            {
                perror("Erro ao enviar resposta");
                continue;
            }
            break;

        case MESSAGE_TEST:
            printf("Comando de teste recebido do cliente.\n");
            response.type = MESSAGE_ACK; // Resposta indicando sucesso
            snprintf((char *)response.data, sizeof(response.data), "Conexão bem-sucedida!");
            // Converte o Packet de resposta em um buffer
            packet_size = build_packet(send_buffer, &response);

            // Envia o buffer de resposta ao cliente
            bytes_sent = send(sock_fd, send_buffer, packet_size, 0);
            if (bytes_sent < 0)
            {
                perror("Erro ao enviar resposta");
                continue;
            }
            printf("Resposta: Tipo %u, Sequência %u\n", response.type, response.seq);
            break;

        default:
            // printf("Comando não reconhecido.\n");
            response.type = MESSAGE_ERROR;
            // Converte o Packet de resposta em um buffer
            packet_size = build_packet(send_buffer, &response);

            // Envia o buffer de resposta ao cliente
            bytes_sent = send(sock_fd, send_buffer, packet_size, 0);
            if (bytes_sent < 0)
            {
                perror("Erro ao enviar resposta");
                continue;
            }
            break;
        }
    }

    close(sock_fd); // Fecha o socket
    return EXIT_SUCCESS;
}
