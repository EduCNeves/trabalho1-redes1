#ifndef UTIL_H
#define UTIL_H

// Define o diretório padrão para operações de backup
#define BACKUP_DIR "backup/"

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
 * Exibe uma tabela com o nome e a data de modificação dos arquivos
 * no diretório definido por BACKUP_DIR.
 */
void list_files();

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

#endif
