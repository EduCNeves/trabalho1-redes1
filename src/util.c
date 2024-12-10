#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>

volatile int running = 1; // Inicializa a variável global

char *get_full_path(const char *filename)
{
    if (!filename)
    {
        return NULL; // Retorna NULL se o caminho ou o nome do arquivo forem inválidos
    }

    // Calcula o tamanho necessário para o caminho completo
    size_t path_len = strlen(BACKUP_DIR);
    size_t filename_len = strlen(filename);
    size_t full_path_len = path_len + filename_len + 2; // +1 para '/' e +1 para o terminador '\0'

    // Aloca memória para a string do caminho completo
    char *full_path = (char *)malloc(full_path_len);
    if (!full_path)
    {
        return NULL; // Retorna NULL se a alocação de memória falhar
    }

    // Monta o caminho completo
    snprintf(full_path, full_path_len, "%s/%s", BACKUP_DIR, filename);

    return full_path;
}

// Função para verificar se o arquivo existe no diretório "backup"
int validate_file(char *file)
{
    char *path = get_full_path(file);

    struct stat buffer;
    // Retorna 1 se o arquivo existir no diretório "backup", 0 caso contrário
    return (stat(path, &buffer) == 0);
}

// Função para solicitar o nome de um arquivo válido no diretório atual
char *get_valid_filename(const char *prompt)
{
    char file[256];
    char *filename;

    while (1)
    {
        printf("%s", prompt);
        fgets(file, sizeof(file), stdin);
        file[strcspn(file, "\n")] = 0; // Remove newline character

        if (validate_file(file))
        {
            // Aloca memória para a string final e copia o nome do arquivo
            filename = (char *)malloc(strlen(file) + 1);
            if (!filename)
            {
                printf("Erro de memória.\n");
                return NULL;
            }
            strcpy(filename, file);
            return filename;
        }
        else if (strcmp(file, "0") == 0)
        {
            printf("Voltando\n");
            return NULL;
        }
        else
        {
            printf("Erro: o arquivo não existe no diretório de backup.\n");
        }
    }
}

void clear_input_buffer()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ; // Descarta caracteres até encontrar '\n' ou EOF
}

char *list_files()
{
    struct dirent *entry;
    struct stat file_stat;
    DIR *dir = opendir(BACKUP_DIR);

    if (!dir)
    {
        perror("Erro ao abrir o diretório de backup");
        return NULL;
    }

    size_t buffer_size = 1024;
    size_t used_size = 0;
    char *result = malloc(buffer_size);
    if (!result)
    {
        perror("Erro ao alocar memória para a lista de arquivos");
        closedir(dir);
        return NULL;
    }
    result[0] = '\0';

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char filepath[1024];
            snprintf(filepath, sizeof(filepath), "%s/%s", BACKUP_DIR, entry->d_name);

            if (stat(filepath, &file_stat) == 0)
            {
                char mod_time[64];
                strftime(mod_time, sizeof(mod_time), "%Y-%m-%d %H:%M:%S", localtime(&file_stat.st_mtime));

                char file_entry[256]; // Tamanho ajustado para evitar warning
                snprintf(file_entry, sizeof(file_entry), "%.128s\t%.128s\n", entry->d_name, mod_time);

                size_t entry_length = strlen(file_entry);
                if (used_size + entry_length + 1 > buffer_size)
                {
                    buffer_size *= 2;
                    char *new_result = realloc(result, buffer_size);
                    if (!new_result)
                    {
                        perror("Erro ao realocar memória");
                        free(result);
                        closedir(dir);
                        return NULL;
                    }
                    result = new_result;
                }

                strcat(result, file_entry);
                used_size += entry_length;
            }
        }
    }

    closedir(dir);
    return result;
}

void handle_signal(int sig)
{
    if (sig == SIGINT)
    {
        printf("\nSinal SIGINT recebido. Encerrando...\n");
        running = 0; // Atualiza o estado para indicar que o programa deve encerrar
    }
}

// Função para imprimir um número em binário
void print_binary(uint8_t value)
{
    for (int i = 7; i >= 0; i--)
    {                                   // Percorre os 8 bits do número
        printf("%d", (value >> i) & 1); // Desloca o bit para a direita e aplica máscara
    }
    printf(" "); // Adiciona espaço para separação
}

long get_file_size(FILE *file_ptr)
{
    if (file_ptr == NULL)
    {
        return -1; // Retorna -1 se o arquivo não estiver aberto
    }

    // Move o ponteiro do arquivo para o final
    fseek(file_ptr, 0, SEEK_END);

    // Obtém a posição atual (tamanho do arquivo)
    long file_size = ftell(file_ptr);

    // Retorna o ponteiro do arquivo para o início
    rewind(file_ptr);

    return file_size; // Retorna o tamanho do arquivo
}
