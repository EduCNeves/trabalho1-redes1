#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MARCADOR_INICIO 0x7E
#define CMD_BACKUP 0x10
#define ACK 0x00
#define NACK 0x01
#define MAX_DADOS 63
#define INTERFACE "wlp0s20f3"

#define CMD_RESTAURA 0x05
#define CMD_VERIFICA 0x06

// Estrutura do pacote
typedef struct {
    unsigned char marcador;
    unsigned char tipo;
    unsigned char tamanho;
    unsigned char sequencia;
    unsigned char dados[MAX_DADOS];
    unsigned char crc;
} Pacote;

// Função para calcular o CRC
unsigned char calcula_crc(Pacote *pacote) {
    unsigned char crc = 0;
    for (int i = 0; i < sizeof(Pacote) - 1; i++) {
        crc ^= ((unsigned char *)pacote)[i];
    }
    return crc;
}

// Função para enviar uma resposta
/*
void envia_resposta(int soquete, struct sockaddr_ll *cliente, socklen_t len, unsigned char tipo_resposta) {
    // Pacote resposta = {0};
    memset(resposta, 0, sizeof(resposta)); // Inicializa a estrutura com zeros

    resposta.marcador = MARCADOR_INICIO;
    resposta.tipo = tipo_resposta; // Deve ser ACK (0x00) ou NACK (0x01)
    resposta.crc = calcula_crc(&resposta);

    if (sendto(soquete, &resposta, sizeof(resposta), 0, (struct sockaddr *)cliente, len) < 0) {
        perror("Erro ao enviar resposta");
    } else {
        printf("Resposta enviada: Tipo=%d\n", tipo_resposta);
    }
}*/

void envia_pacote(int soquete, Pacote *pacote, struct sockaddr_ll *cliente, socklen_t len) {
    ssize_t enviado = sendto(
        soquete,                  // Socket de envio
        pacote,                   // Ponteiro para o pacote
        sizeof(Pacote),           // Tamanho total do pacote
        0,                        // Flags (geralmente 0)
        (struct sockaddr *)cliente, // Endereço do cliente
        len                       // Tamanho da estrutura de endereço
    );

    if (enviado == -1) {
        perror("Erro ao enviar pacote");
    } else {
        printf("Pacote enviado: %ld bytes.\n", enviado);
    }
}

void monta_pacote(Pacote *pacote, unsigned char tipo, unsigned char sequencia, unsigned char *dados, int tamanho_dados) {
    memset(pacote, 0, sizeof(Pacote)); // Inicializa a estrutura com zeros

    pacote->marcador = MARCADOR_INICIO;      // Marcador para identificar início do pacote
    pacote->tipo = tipo;                     // Tipo do comando (ACK, NACK, etc.)
    pacote->sequencia = sequencia;           // Número de sequência
    pacote->tamanho = tamanho_dados;         // Tamanho dos dados
    if (tamanho_dados > 0) {
        memcpy(pacote->dados, dados, tamanho_dados); // Copia os dados
    }
    pacote->crc = calcula_crc(pacote);       // Calcula o CRC (ou outro checksum)
}

void envia_resposta(int soquete, struct sockaddr_ll *cliente, socklen_t len, unsigned char tipo_resposta) {
    Pacote resposta;
    monta_pacote(&resposta, tipo_resposta, 0, NULL, 0); // Cria pacote de ACK ou NACK
    envia_pacote(soquete, &resposta, cliente, len);     // Envia a resposta
}

void processa_pacote(int soquete, Pacote *pacote, struct sockaddr_ll *cliente, socklen_t len) {
    
    envia_resposta(soquete, cliente, len, ACK);
    /*
    static unsigned char ultima_sequencia = 255; // Para evitar pacotes repetidos

    // Verifica CRC
    if (calcula_crc(pacote) != pacote->crc) {
        printf("CRC inválido. Pacote ignorado.\n");
        envia_resposta(soquete, cliente, len, NACK);
        return;
    }

    // Verifica pacotes repetidos
    if (pacote->sequencia == ultima_sequencia) {
        printf("Pacote repetido (sequência %d). Ignorando.\n", pacote->sequencia);
        return;
    }
    ultima_sequencia = pacote->sequencia;

    // Processa diferentes tipos de pacotes
    switch (pacote->tipo) {
        case CMD_BACKUP:
            printf("Recebido comando de backup. Dados: %s\n", pacote->dados);
            // Aqui você pode implementar lógica para salvar um arquivo.
            break;
        case CMD_RESTAURA:
            printf("Recebido comando de restauração. Dados: %s\n", pacote->dados);
            // Aqui você pode implementar lógica para enviar um arquivo ao cliente.
            break;
        case CMD_VERIFICA:
            printf("Recebido comando de verificação. Dados: %s\n", pacote->dados);
            // Aqui você pode implementar lógica para verificar o checksum de um arquivo.
            break;
        default:
            printf("Comando desconhecido. Tipo: %d\n", pacote->tipo);
            envia_resposta(soquete, cliente, len, NACK);
            return;
    }

    // Envia ACK para pacotes válidos e processados
    printf("Pacote processado com sucesso. Enviando ACK.\n");
    envia_resposta(soquete, cliente, len, ACK);
    */
    printf("Pacote processado com sucesso. Enviando ACK.\n");

    
}



// Função principal do servidor
int main() {
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (soquete == -1) {
        perror("Erro ao criar socket");
        return -1;
    }

    struct sockaddr_ll endereco_servidor, endereco_cliente;
    socklen_t len = sizeof(endereco_cliente);
    
    Pacote pacote; // ver issoo

    // Configura a interface de rede
    memset(&endereco_servidor, 0, sizeof(endereco_servidor));
    endereco_servidor.sll_family = AF_PACKET;
    endereco_servidor.sll_protocol = htons(ETH_P_ALL);
    int indice_interface = if_nametoindex(INTERFACE);
    if (indice_interface == 0) {
        perror("Erro ao obter índice da interface");
        close(soquete);
        return -1;
    }
    endereco_servidor.sll_ifindex = indice_interface;


    if (bind(soquete, (struct sockaddr *)&endereco_servidor, sizeof(endereco_servidor)) < 0) {
        perror("Erro ao fazer bind");
        close(soquete);
        exit(EXIT_FAILURE);
    }

    printf("Servidor aguardando pacotes...\n");

    while (1) {
        // Recebe pacotes do cliente
        ssize_t recebido = recvfrom(soquete, &pacote, sizeof(pacote), 0, (struct sockaddr *)&endereco_cliente, &len);
        if (recebido > 0) {
            if (pacote.marcador == MARCADOR_INICIO) {
                printf("Pacote recebido (%ld bytes). Processando...\n", recebido);
                processa_pacote(soquete, &pacote, &endereco_cliente, len);
            }
            //else {
                // printf("Pacote inválido (Marcador errado)\n");
            // }
        } else {
            perror("Erro ao receber pacote");
        }
    }

    close(soquete);
    return 0;

}
