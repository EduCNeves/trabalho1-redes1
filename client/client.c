#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commands.h"
#include "util.h"

int main()
{
    int choice;
    char *file;

    while (1)
    {
        // Exibe o menu de opções
        printf("\nMenu:\n");
        printf("1. Backup\n");
        printf("2. Restore\n");
        printf("3. Check\n");
        printf("4. List files in backup directory\n");
        printf("5. Exit\n");
        printf("Escolha sua tarefa 1-5): ");

        if (scanf("%d", &choice) != 1)
        {
            choice = -1; // Define choice como inválido se a entrada não for um número
        }

        clear_input_buffer(); // Limpa o buffer após uma entrada válida ou inválida

        CommandError e;

        switch (choice)
        {
        case 1: // Backup
            file = get_valid_filename("Digite o nome de arquivo para backup: ");

            e = backup(file);
            if (e != COMMAND_SUCCESS)
            {
                handle_command_error(e);
            }

            free(file);
            break;

        case 2: // Restore
            char f[256];
            printf("Digite o nome de arquivo para restaurar: ");
            fgets(f, sizeof(f), stdin);
            f[strcspn(f, "\n")] = 0; // Remove newline character

            e = restore(f);
            if (e != COMMAND_SUCCESS)
            {
                handle_command_error(e);
            }
            break;

        case 3: // Check
            file = get_valid_filename("Digite o nome de arquivo para verificar: ");

            e = check(file);
            if (e != COMMAND_SUCCESS)
            {
                handle_command_error(e);
            }

            free(file);
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
