#include "util.h"
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#define BACKUP_DIR "backup/"

// Função para verificar se o arquivo existe no diretório "backup"
int validate_file(char *file) {
    char path[512];
    snprintf(path, sizeof(path), "%s%s", BACKUP_DIR, file);

    struct stat buffer;
    // Retorna 1 se o arquivo existir no diretório "backup", 0 caso contrário
    return (stat(path, &buffer) == 0);
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
