#include "commands.h"
#include "util.h"
#include "connection.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Função para backup
CommandError backup(const char *file)
{
    if (!file)
    {
        return COMMAND_INVALID_INPUT;
    }

    char *path = get_full_path(file);

    FILE *file_ptr = fopen(path, "rb");
    free(path); // Libera o caminho após o uso
    if (!file_ptr)
    {
        return COMMAND_FILE_NOT_FOUND;
    }

    // ENVIA COMMAND BACKUP

    Command cmd;
    CommandError error = convert_to_command(&cmd, COMMAND_BACKUP, file, NULL, 0, COMMAND);
    if (error != COMMAND_SUCCESS)
    {
        fclose(file_ptr);
        return error;
    }

    // Converte o comando para uma fila de pacotes
    PacketQueue *queue = convert_command_to_packets(&cmd);
    if (!queue)
    {
        free(cmd.data.string_data);
        return COMMAND_MEMORY_ERROR;
    }

    // Envia a fila de pacotes e recebe a resposta
    PacketQueue *response_queue = send_packet_queue(queue);
    if (!response_queue)
    {
        return COMMAND_COMMUNICATION_ERROR;
    }

    // Converte a resposta em um comando
    Command response_cmd = convert_packets_to_command(response_queue);
    free_packet_queue(response_queue); // Libera a fila de resposta

    if (response_cmd.type == MESSAGE_ERROR)
    {
        return COMMAND_SERVER_ERROR;
    }

    // Exibe a resposta do servidor
    if (response_cmd.type != MESSAGE_OK)
        return COMMAND_COMMUNICATION_ERROR;

    // ENVIA SIZE

    long size = get_file_size(file_ptr);
    if (size < 0)
    {
        return COMMAND_INVALID_INPUT;
    }

    error = convert_to_command(&cmd, MESSAGE_SIZE, NULL, NULL, size, SIZE);
    if (error != COMMAND_SUCCESS)
    {
        fclose(file_ptr);
        return error;
    }

    // Converte o comando para uma fila de pacotes
    queue = convert_command_to_packets(&cmd);
    if (!queue)
    {
        free(cmd.data.string_data);
        return COMMAND_MEMORY_ERROR;
    }

    // Envia a fila de pacotes e recebe a resposta
    response_queue = send_packet_queue(queue);
    if (!response_queue)
    {
        return COMMAND_COMMUNICATION_ERROR;
    }

    // Converte a resposta em um comando
    response_cmd = convert_packets_to_command(response_queue);
    free_packet_queue(response_queue); // Libera a fila de resposta

    if (response_cmd.type == MESSAGE_ERROR)
    {
        return COMMAND_SERVER_ERROR;
    }

    // Exibe a resposta do servidor
    if (response_cmd.type != MESSAGE_OK)
        return COMMAND_COMMUNICATION_ERROR;

    // ENVIA FILE

    error = convert_to_command(&cmd, MESSAGE_DATA, NULL, file_ptr, 0, FILE_PTR);
    if (error != COMMAND_SUCCESS)
    {
        fclose(file_ptr);
        return error;
    }

    // Converte o comando para uma fila de pacotes
    queue = convert_command_to_packets(&cmd);
    if (!queue)
    {
        free(cmd.data.string_data);
        return COMMAND_MEMORY_ERROR;
    }

    // Envia a fila de pacotes e recebe a resposta
    response_queue = send_packet_queue(queue);
    if (!response_queue)
    {
        return COMMAND_COMMUNICATION_ERROR;
    }

    // Converte a resposta em um comando
    response_cmd = convert_packets_to_command(response_queue);
    free_packet_queue(response_queue); // Libera a fila de resposta

    if (response_cmd.type == MESSAGE_ERROR)
    {
        return COMMAND_SERVER_ERROR;
    }

    // Exibe a resposta do servidor
    if (response_cmd.type != MESSAGE_ACK)
        return COMMAND_COMMUNICATION_ERROR;

    printf("tudo certo até aqui bb\n");

    fclose(file_ptr); // Fecha o arquivo após o uso

    free_packet_queue(queue); // Libera a fila enviada

    return COMMAND_SUCCESS;

    // PacketQueue *send_queue = convert_command_to_packets(&cmd);

    // PacketQueue *response_queue = send_packet_queue(send_queue);

    // free_packet_queue(send_queue);

    // cmd = convert_packets_to_command(response_queue);

    // free_packet_queue(response_queue);
}

// Função para restore
CommandError restore(const char *file)
{
    if (!file)
    {
        return COMMAND_INVALID_INPUT;
    }

    // Obter o caminho completo do arquivo
    char *path = get_full_path(file);
    if (!path)
    {
        return COMMAND_MEMORY_ERROR;
    }

    Command cmd;
    CommandError error = convert_to_command(&cmd, COMMAND_RESTORE, path, NULL, 0, COMMAND);
    free(path); // Libera o caminho após o uso
    if (error != COMMAND_SUCCESS)
    {
        return error;
    }

    // Lógica de envio de pacotes seria implementada aqui
    printf("tudo certo até aqui\n");

    free(cmd.data.string_data); // Libera a memória alocada para a string
    return COMMAND_SUCCESS;
}

// Função para check
CommandError check(const char *file)
{
    if (!file)
    {
        return COMMAND_INVALID_INPUT;
    }

    // Obter o caminho completo do arquivo
    char *path = get_full_path(file);
    if (!path)
    {
        return COMMAND_MEMORY_ERROR;
    }

    FILE *file_ptr = fopen(path, "rb");
    free(path); // Libera o caminho após o uso
    if (!file_ptr)
    {
        return COMMAND_FILE_NOT_FOUND;
    }

    Command cmd;
    CommandError error = convert_to_command(&cmd, COMMAND_CHECK, NULL, file_ptr, 0, FILE_PTR);
    if (error != COMMAND_SUCCESS)
    {
        fclose(file_ptr);
        return error;
    }

    // Lógica de envio de pacotes seria implementada aqui
    printf("tudo certo até aqui\n");

    fclose(file_ptr); // Fecha o arquivo após o uso
    return COMMAND_SUCCESS;
}

CommandError convert_to_command(Command *cmd, MessageType type, const char *string_data, FILE *file_data, long long_data, DataType data_type)
{
    // Validação de entrada
    if (!cmd || (!string_data && !file_data && long_data < 0))
    {
        return COMMAND_INVALID_INPUT;
    }

    // Define o tipo de comando/mensagem
    cmd->type = type;

    // Define o tipo de dado e carrega o conteúdo correspondente
    cmd->data_type = data_type;

    if (data_type == SIZE)
    {
        cmd->data.long_data = long_data;
    }
    else if (data_type == COMMAND)
    {
        // Aloca memória para a string e copia o dado
        cmd->data.string_data = strdup(string_data);
        if (!cmd->data.string_data)
        {
            return COMMAND_MEMORY_ERROR;
        }
    }
    else if (data_type == FILE_PTR)
    {
        // Atribui o ponteiro de arquivo diretamente
        cmd->data.file_data = file_data;
        if (!cmd->data.file_data)
        {
            return COMMAND_FILE_NOT_FOUND;
        }
    }

    return COMMAND_SUCCESS;
}

void handle_command_error(CommandError error_code)
{
    switch (error_code)
    {
    case COMMAND_SUCCESS:
        printf("Command executed successfully.\n");
        break;
    case COMMAND_FILE_NOT_FOUND:
        printf("Error: File not found.\n");
        break;
    case COMMAND_INVALID_INPUT:
        printf("Error: Invalid input.\n");
        break;
    case COMMAND_MEMORY_ERROR:
        printf("Error: Memory allocation failed.\n");
        break;
    default:
        printf("Error: Unknown error occurred.\n");
    }
}

CommandError list_server_files()
{
    // Prepara o comando de teste
    Command cmd;
    memset(&cmd, 0, sizeof(Command));
    cmd.type = COMMAND_LIST_FILES;
    cmd.data_type = COMMAND;
    cmd.data.string_data = NULL;

    // Converte o comando para uma fila de pacotes
    PacketQueue *queue = convert_command_to_packets(&cmd);
    if (!queue)
    {
        free(cmd.data.string_data);
        return COMMAND_MEMORY_ERROR;
    }

    // Envia a fila de pacotes e recebe a resposta
    PacketQueue *response_queue = send_packet_queue(queue);

    if (!response_queue)
    {
        return COMMAND_COMMUNICATION_ERROR;
    }

    // Converte a resposta em um comando
    Command response_cmd = convert_packets_to_command(response_queue);
    free_packet_queue(response_queue); // Libera a fila de resposta

    if (response_cmd.type == MESSAGE_ERROR)
    {
        return COMMAND_SERVER_ERROR;
    }

    // Exibe a resposta do servidor
    if (response_cmd.data_type == COMMAND && response_cmd.type == MESSAGE_OK)
    {
        PacketQueue *list = receive_data();
        Command list_cmd = convert_packets_to_command(list);

        printf("\nArquivos no servidor:\n");
        printf("%s", list_cmd.data.string_data);

        free(list_cmd.data.string_data);
        free_packet_queue(queue); // Libera a fila enviada

        return COMMAND_SUCCESS;
    }

    free_packet_queue(queue); // Libera a fila enviada

    return COMMAND_SUCCESS;
}

CommandError test_connection()
{
    // Prepara o comando de teste
    Command cmd;
    memset(&cmd, 0, sizeof(Command));
    cmd.type = MESSAGE_TEST;
    cmd.data_type = COMMAND;
    cmd.data.string_data = NULL;

    // Converte o comando para uma fila de pacotes
    PacketQueue *queue = convert_command_to_packets(&cmd);
    if (!queue)
    {
        free(cmd.data.string_data);
        return COMMAND_MEMORY_ERROR;
    }

    // Envia a fila de pacotes e recebe a resposta
    PacketQueue *response_queue = send_packet_queue(queue);
    free_packet_queue(queue); // Libera a fila enviada

    if (!response_queue)
    {
        return COMMAND_COMMUNICATION_ERROR;
    }

    // Converte a resposta em um comando
    Command response_cmd = convert_packets_to_command(response_queue);
    free_packet_queue(response_queue); // Libera a fila de resposta

    if (response_cmd.type == MESSAGE_ERROR)
    {
        return COMMAND_SERVER_ERROR;
    }

    // Exibe a resposta do servidor
    if (response_cmd.data_type == COMMAND && response_cmd.type == MESSAGE_ACK)
    {
        printf("Resposta do servidor: ACK\n");
    }
    else
    {
        return COMMAND_INVALID_RESPONSE;
    }

    return COMMAND_SUCCESS;
}
