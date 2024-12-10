#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "../src/commands.h"
#include "../src/util.h"
#include "../src/connection.h"
#include "../src/util.h"

int main()
{
    int choice;
    char *file;
    // Configura o manipulador de sinal para encerrar o cliente
    signal(SIGINT, handle_signal);

    // Inicializa a conexão como cliente
    int sock_fd = initialize_connection(0); // 1 indica que é cliente
    if (sock_fd < 0)
    {
        fprintf(stderr, "Erro ao inicializar o servidor.\n");
        return EXIT_FAILURE;
    }

    printf("Testando a conexão com o servidor...\n");
    CommandError err = test_connection();
    if (err != COMMAND_SUCCESS)
    {
        handle_command_error(err);
    }

    while (running)
    {
        // Exibe o menu de opções
        printf("\nMenu:\n");
        printf("1. Backup\n");
        printf("2. Restore\n");
        printf("3. Check\n");
        printf("4. List files in backup directory\n");
        printf("5. List files in Server\n");
        printf("6. Teste de conexão\n");
        printf("0. Exit\n");
        printf("Escolha sua tarefa (0-6): ");

        if (scanf("%d", &choice) != 1)
        {
            choice = -1; // Define choice como inválido se a entrada não for um número
        }

        clear_input_buffer(); // Limpa o buffer após uma entrada válida ou inválida

        CommandError e;

        switch (choice)
        {
        case 1: // Backup
            file = get_valid_filename("Digite o nome de arquivo para backup (0 para voltar): ");

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
            file = get_valid_filename("Digite o nome de arquivo para verificar (0 para voltar): ");

            e = check(file);
            if (e != COMMAND_SUCCESS)
            {
                handle_command_error(e);
            }

            free(file);
            break;

        case 4: // List files
            printf("%s", list_files());
            break;

        case 5:
            list_server_files();
            break;

        case 6: // Teste de conexão
            printf("Testando a conexão com o servidor...\n");
            CommandError err = test_connection();
            if (err != COMMAND_SUCCESS)
            {
                handle_command_error(err);
            }
            break;

        case 0: // Exit
            printf("Saindo do programa...\n");
            return 0;

        default:
            printf("Escolha invalida. Escolha um numero entre 0 e 6.\n");
        }
    }

    close(sock_fd); // Fecha o socket

    return 0;
}
