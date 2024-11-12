#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "util.h"

int main() {
    int choice;
    char file[256];

    while (1) {
        // Exibe o menu de opções
        printf("\nMenu:\n");
        printf("1. Backup\n");
        printf("2. Restore\n");
        printf("3. Check\n");
        printf("4. List files in backup directory\n");
        printf("5. Exit\n");
        printf("Escolha sua tarefa 1-5): ");
        scanf("%d", &choice);
        getchar(); // Para consumir o caractere de nova linha após scanf

        switch (choice) {
            case 1: // Backup
                printf("Digite o nome de arquivo para backup: ");
                fgets(file, sizeof(file), stdin);
                file[strcspn(file, "\n")] = 0; // Remove newline character
                while (!validate_file(file)) {
                    printf("Erro: o arquivo não existe no diretorio atual.\n");
                    printf("Digite um nome valido de arquivo para backup: ");
                    fgets(file, sizeof(file), stdin);
                    file[strcspn(file, "\n")] = 0; // Remove newline character
                }
                backup(file);
                break;

            case 2: // Restore
                printf("Digite o nome de arquivo para restaurar: ");
                fgets(file, sizeof(file), stdin);
                file[strcspn(file, "\n")] = 0; // Remove newline character
                restore(file);
                break;

            case 3: // Check
                printf("Digite o nome de arquivo para verificar: ");
                fgets(file, sizeof(file), stdin);
                file[strcspn(file, "\n")] = 0; // Remove newline character
                check(file);
                break;

            case 4: // List files
                list_files();
                break;

            case 5: // Exit
                printf("Saindo do programa...\n");
                return 0;

            default:
                printf("Escolha invalida. Escolha um numero entre 1 e 4.\n");
        }
    }

    return 0;
}
