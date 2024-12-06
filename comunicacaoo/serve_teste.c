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

typedef struct {
    unsigned char marcador;
    unsigned char tipo;
    unsigned char tamanho;
    unsigned char sequencia;
    unsigned char dados[MAX_DADOS];
    unsigned char crc;
} Pacote;

unsigned char calcula_crc(Pacote *pacote) {
    unsigned char crc = 0;
    for (int i = 0; i < sizeof(Pacote) - 1; i++) {
        crc ^= ((unsigned char *)pacote)[i];
    }
    return crc;
}

void monta_pacote(Pacote *pacote, unsigned char tipo, unsigned char sequencia, unsigned char *dados, int tamanho_dados) {
    memset(pacote, 0, sizeof(Pacote));
    pacote->marcador = MARCADOR_INICIO;
    pacote->tipo = tipo;
    pacote->sequencia = sequencia;
    pacote->tamanho = tamanho_dados;
    if (tamanho_dados > 0) {
        memcpy(pacote->dados, dados, tamanho_dados);
    }
    pacote->crc = calcula_crc(pacote);
}

void envia_resposta(int soquete, struct sockaddr_ll *cliente, socklen_t len, unsigned char tipo_resposta) {
    Pacote resposta;
    monta_pacote(&resposta, tipo_resposta, 0, NULL, 0);
    ssize_t enviado = sendto(soquete, &resposta, sizeof(Pacote), 0, (struct sockaddr *)cliente, len);
    if (enviado < 0) {
        perror("Erro ao enviar resposta");
    } else {
        printf("Resposta enviada: Tipo=%d\n", tipo_resposta);
    }
}

void processa_pacote(int soquete, Pacote *pacote, struct sockaddr_ll *cliente, socklen_t len) {
    
    if (calcula_crc(pacote) != pacote->crc) {
        printf("CRC inválido. Enviando NACK.\n");
        envia_resposta(soquete, cliente, len, NACK);
        return;
    }

    static unsigned char ultima_sequencia = 255; // Para evitar pacotes repetidos

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

    printf("Pacote recebido: Tipo=%d, Dados=%s\n", pacote->tipo, pacote->dados);
    envia_resposta(soquete, cliente, len, ACK);
}

int main() {
    
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (soquete < 0) {
        perror("Erro ao criar socket");
        return -1;
    }

    struct sockaddr_ll endereco_servidor, endereco_cliente;
    socklen_t len = sizeof(endereco_cliente);
    Pacote pacote;

    memset(&endereco_servidor, 0, sizeof(endereco_servidor));
    endereco_servidor.sll_family = AF_PACKET;
    endereco_servidor.sll_protocol = htons(ETH_P_ALL);
    endereco_servidor.sll_ifindex = if_nametoindex(INTERFACE);

    if (endereco_servidor.sll_ifindex == 0) {
        perror("Erro ao obter índice da interface");
        close(soquete);
        return -1;
    }

    if (bind(soquete, (struct sockaddr *)&endereco_servidor, sizeof(endereco_servidor)) < 0) {
        perror("Erro ao fazer bind");
        close(soquete);
        return -1;
    }

    printf("Servidor aguardando pacotes...\n");

    while (1) {
        ssize_t recebido = recvfrom(soquete, &pacote, sizeof(Pacote), 0, (struct sockaddr *)&endereco_cliente, &len);
        if (recebido > 0) {
            if (pacote.marcador == MARCADOR_INICIO) {
                processa_pacote(soquete, &pacote, &endereco_cliente, len);
            } else {
                // printf("Pacote inválido: marcador errado.\n");
            }
        } else {
            perror("Erro ao receber pacote");
        }
    }

    close(soquete);
    return 0;
}
