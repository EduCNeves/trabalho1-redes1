#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>

#ifndef PACKET_H
#include "packet.h" // Para Packet e PacketQueue
#endif

// Enum para representar os tipos de mensagem baseados na tabela fornecida
typedef enum
{
    // Mensagens de controle
    MESSAGE_ACK = 0b00000,
    MESSAGE_NACK = 0b00001,
    MESSAGE_OK = 0b00010,
    MESSAGE_OK_CHECKSUM = 0b01101,
    MESSAGE_OK_SIZE = 0b01110,
    MESSAGE_SIZE = 0b01111,
    MESSAGE_DATA = 0b10000,
    MESSAGE_END_DATA = 0b10001,
    MESSAGE_ERROR = 0b11111,

    // Códigos de comando
    COMMAND_BACKUP = 0b10100,
    COMMAND_RESTORE = 0b10101,
    COMMAND_CHECK = 0b10110,

    COMMAND_LIST_FILES = 0b10111,
    MESSAGE_LIST_FILES = 0b11001,
    MESSAGE_TEST = 0b11000
} MessageType;

/**
 * @brief Enum para representar o tipo de dado usado em um comando.
 *
 * STRING representa um dado do tipo string.
 * FILE_PTR representa um dado do tipo FILE pointer.
 */
typedef enum
{
    COMMAND,
    FILE_PTR,
    SIZE
} DataType;

/**
 * @brief Enum para representar códigos de erro possíveis nas operações de comando.
 *
 * COMMAND_SUCCESS indica que o comando foi executado com sucesso.
 * COMMAND_FILE_NOT_FOUND indica que o arquivo especificado não foi encontrado.
 * COMMAND_INVALID_INPUT indica uma entrada inválida.
 * COMMAND_MEMORY_ERROR indica uma falha de alocação de memória.
 */
typedef enum
{
    COMMAND_SUCCESS,
    COMMAND_FILE_NOT_FOUND,
    COMMAND_INVALID_INPUT,
    COMMAND_MEMORY_ERROR,
    COMMAND_COMMUNICATION_ERROR,
    COMMAND_SERVER_ERROR,
    COMMAND_INVALID_RESPONSE
} CommandError;

/**
 * @brief Estrutura para esse packet vou adcionar duas funções para representar um comando, incluindo o nome do comando e o dado associado.
 *
 * command_name armazena o nome do comando.
 * data_type indica o tipo de dado associado ao comando.
 * data é uma união que armazena o dado, podendo ser uma string ou um FILE pointer.
 */
typedef struct
{
    MessageType type;
    DataType data_type;
    union
    {
        char *string_data;
        FILE *file_data;
        long long_data;
    } data;
} Command;

/**
 * @brief Executa o comando de backup de um arquivo.
 *
 * @param file Nome do arquivo a ser enviado para backup.
 * @return CommandError Código de erro indicando o resultado da operação.
 */
CommandError backup(const char *file);

/**
 * @brief Executa o comando de restauração de um arquivo.
 *
 * @param file Nome do arquivo a ser restaurado.
 * @return CommandError Código de erro indicando o resultado da operação.
 */
CommandError restore(const char *file);

/**
 * @brief Executa o comando de verificação de um arquivo.
 *
 * @param file Nome do arquivo a ser verificado.
 * @return CommandError Código de erro indicando o resultado da operação.
 */
CommandError check(const char *file);

/**
 * @brief Exibe uma mensagem de erro com base no código de erro fornecido.
 *
 * @param error_code Código de erro que será tratado e exibido.
 */
void handle_command_error(CommandError error_code);

/**
 * @brief Converte dados de entrada em uma estrutura Command.
 *
 * @param cmd Ponteiro para a estrutura Command a ser inicializada.
 * @param type Tipo da mensagem ou comando, representado pelo enum MessageType (ex.: COMMAND_BACKUP, COMMAND_RESTORE).
 * @param string_data Dado do tipo string (ou NULL se não for usado).
 * @param file_data Ponteiro para FILE (ou NULL se não for usado).
 * @param long_data Dados em long int (ou NULL se não for usado).
 * @param data_type Tipo de dado, especificando se o conteúdo é uma string ou um arquivo (STRING ou FILE_PTR).
 * @return CommandError Código de erro indicando o resultado da operação.
 */
CommandError convert_to_command(Command *cmd, MessageType type, const char *string_data, FILE *file_data, long long_data, DataType data_type);

/**
 * @brief Solicita a lista de arquivos do servidor e exibe as informações.
 *
 * Envia um comando ao servidor para listar os arquivos no diretório de backup.
 * A resposta do servidor é exibida no formato de uma tabela com o nome e a data
 * de modificação dos arquivos.
 *
 * @return CommandError Código de erro indicando o resultado da operação.
 *         - COMMAND_SUCCESS: Operação concluída com sucesso.
 *         - COMMAND_MEMORY_ERROR: Falha na alocação de memória.
 *         - COMMAND_COMMUNICATION_ERROR: Erro na comunicação com o servidor.
 *         - COMMAND_SERVER_ERROR: O servidor retornou uma resposta de erro.
 *         - COMMAND_INVALID_RESPONSE: Resposta do servidor inválida ou inesperada.
 */
CommandError list_server_files();

/**
 * @brief Testa a conexão com o servidor enviando um comando simples.
 *
 * @return CommandError Código de erro indicando o resultado da operação.
 */
CommandError test_connection();

#endif
