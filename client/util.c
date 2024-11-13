#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

char *get_full_path(const char *filename) {
    if (!filename) {
        return NULL; // Retorna NULL se o caminho ou o nome do arquivo forem inválidos
    }

    // Calcula o tamanho necessário para o caminho completo
    size_t path_len = strlen(BACKUP_DIR);
    size_t filename_len = strlen(filename);
    size_t full_path_len = path_len + filename_len + 2; // +1 para '/' e +1 para o terminador '\0'

    // Aloca memória para a string do caminho completo
    char *full_path = (char *)malloc(full_path_len);
    if (!full_path) {
        return NULL; // Retorna NULL se a alocação de memória falhar
    }

    // Monta o caminho completo
    snprintf(full_path, full_path_len, "%s/%s", BACKUP_DIR, filename);

    return full_path;
}


// Função para verificar se o arquivo existe no diretório "backup"
int validate_file(char *file) {
    char *path = get_full_path(file);

    struct stat buffer;
    // Retorna 1 se o arquivo existir no diretório "backup", 0 caso contrário
    return (stat(path, &buffer) == 0);
}

// Função para solicitar o nome de um arquivo válido no diretório atual
char *get_valid_filename(const char *prompt) {
    char file[256];
    char *filename;

    while (1) {
        printf("%s", prompt);
        fgets(file, sizeof(file), stdin);
        file[strcspn(file, "\n")] = 0; // Remove newline character

        if (validate_file(file)) {
            // Aloca memória para a string final e copia o nome do arquivo
            filename = (char *)malloc(strlen(file) + 1);
            if (!filename) {
                printf("Erro de memória.\n");
                return NULL;
            }
            strcpy(filename, file);
            return filename;
        } else {
            printf("Erro: o arquivo não existe no diretório de backup.\n");
        }
    }
}

void clear_input_buffer() {
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF); // Descarta caracteres até encontrar '\n' ou EOF
}

// Função para listar os arquivos no diretório "backup" com a data de modificação
void list_files() {
    struct dirent *entry;
    struct stat file_stat;
    DIR *dir = opendir(BACKUP_DIR);

    if (dir == NULL) {
        printf("Could not open backup directory.\n");
        return;
    }

    printf("\n%-20s %s\n", "Nome do Arquivo", "Data de Modificação");
    printf("%-20s %s\n", "-----------------", "-------------------");

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Verifica se é um arquivo regular
            char path[512];
            snprintf(path, sizeof(path), "%s%s", BACKUP_DIR, entry->d_name);

            if (stat(path, &file_stat) == 0) {
                // Converte o tempo de modificação para um formato legível
                struct tm *mod_time = localtime(&file_stat.st_mtime);
                char time_str[20];
                strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", mod_time);

                printf("%-20s %s\n", entry->d_name, time_str);
            }
        }
    }

    closedir(dir);
}
