#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdio.h>
#include "connection.h"

/**
 * @brief Enum para representar o tipo de dado usado em um comando.
 * 
 * STRING representa um dado do tipo string.
 * FILE_PTR representa um dado do tipo FILE pointer.
 */
typedef enum {
    STRING,
    FILE_PTR
} DataType;

/**
 * @brief Enum para representar códigos de erro possíveis nas operações de comando.
 * 
 * COMMAND_SUCCESS indica que o comando foi executado com sucesso.
 * COMMAND_FILE_NOT_FOUND indica que o arquivo especificado não foi encontrado.
 * COMMAND_INVALID_INPUT indica uma entrada inválida.
 * COMMAND_MEMORY_ERROR indica uma falha de alocação de memória.
 */
typedef enum {
    COMMAND_SUCCESS,
    COMMAND_FILE_NOT_FOUND,
    COMMAND_INVALID_INPUT,
    COMMAND_MEMORY_ERROR
} CommandError;

/**
 * @brief Estrutura para representar um comando, incluindo o nome do comando e o dado associado.
 * 
 * command_name armazena o nome do comando.
 * data_type indica o tipo de dado associado ao comando.
 * data é uma união que armazena o dado, podendo ser uma string ou um FILE pointer.
 */
typedef struct {
    MessageType type;
    DataType data_type;
    union {
        char *string_data;
        FILE *file_data;
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
 * @param data_type Tipo de dado, especificando se o conteúdo é uma string ou um arquivo (STRING ou FILE_PTR).
 * @return CommandError Código de erro indicando o resultado da operação.
 */
CommandError convert_to_command(Command *cmd, MessageType type, const char *string_data, FILE *file_data, DataType data_type);


#endif
