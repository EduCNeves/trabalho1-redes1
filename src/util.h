#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdio.h>

#include "../config.h"

/**
 * @brief Variável global para controlar o estado de execução.
 *        Deve ser inicializada em um arquivo fonte.
 */
extern volatile int running;

/**
 * @brief Valida se um arquivo existe no diretório especificado.
 *
 * @param file Nome do arquivo a ser validado.
 * @return Retorna 1 se o arquivo existe, 0 caso contrário.
 */
int validate_file(char *file);

/**
 * @brief Lista todos os arquivos no diretório de backup com suas datas de modificação.
 *
 * Retorna uma string formatada contendo o nome e a data de modificação
 * dos arquivos no diretório definido por BACKUP_DIR. Cada linha contém o
 * nome do arquivo seguido pela data de modificação.
 *
 * @return char* Ponteiro para a string contendo a lista de arquivos, ou NULL em caso de erro.
 *         - A string retornada deve ser liberada pelo chamador usando `free`.
 *         - Retorna NULL se o diretório não puder ser aberto ou ocorrer falha de memória.
 */
char *list_files();

/**
 * @brief Gera o caminho completo para um arquivo no diretório de backup.
 *
 * @param filename Nome do arquivo.
 * @return Retorna uma string contendo o caminho completo (diretório + nome do arquivo).
 * A string retornada deve ser liberada com `free` após o uso.
 */
char *get_full_path(const char *filename);

/**
 * @brief Solicita ao usuário um nome de arquivo que deve existir no diretório atual.
 *
 * @param prompt Mensagem de solicitação exibida ao usuário.
 * @return Retorna uma string contendo o nome do arquivo válido.
 * A string retornada deve ser liberada com `free` após o uso.
 */
char *get_valid_filename(const char *prompt);

/**
 * @brief Limpa o buffer de entrada para evitar que entradas extras afetem futuras leituras.
 *
 * Remove todos os caracteres no buffer até encontrar um caractere de nova linha (`\n`)
 * ou fim de arquivo (`EOF`). Usado principalmente após `scanf` para evitar problemas
 * com entradas inválidas.
 */
void clear_input_buffer();

/**
 * @brief Manipula sinais recebidos pelo programa, como SIGINT.
 *
 * Essa função é usada para tratar sinais, permitindo ações específicas, como
 * encerrar o programa de maneira controlada ao receber um sinal SIGINT.
 *
 * @param sig Número do sinal recebido (ex.: SIGINT).
 */
void handle_signal(int sig);

// Função para imprimir um número em binário
void print_binary(uint8_t value);

/**
 * @brief Obtém o tamanho de um arquivo aberto.
 *
 * @param file_ptr Ponteiro para o arquivo aberto em modo binário.
 * @return long Tamanho do arquivo em bytes, ou -1 em caso de erro.
 */
long get_file_size(FILE *file_ptr);

#endif
