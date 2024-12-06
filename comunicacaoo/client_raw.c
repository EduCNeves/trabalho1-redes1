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
#include <errno.h>

#define MARCADOR_INICIO 0x7E
#define CMD_BACKUP 0x10
#define ACK 0x00
#define NACK 0x01
#define MAX_DADOS 63
#define INTERFACE "wlp0s20f3" // Substitua pelo nome da sua interface

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
    unsigned char *dados_pacote = (unsigned char *)pacote;
    for (int i = 0; i < sizeof(Pacote) - 1; i++) {
        crc ^= dados_pacote[i];
    }
    return crc;
}

void monta_pacote(Pacote *pacote, unsigned char tipo, unsigned char sequencia, unsigned char *dados, int tamanho_dados) {
    memset(pacote, 0, sizeof(Pacote));
    pacote->marcador = MARCADOR_INICIO;
    pacote->tipo = tipo;
    pacote->sequencia = sequencia;
    pacote->tamanho = tamanho_dados;
    if (tamanho_dados > 0 && dados != NULL) {
        memcpy(pacote->dados, dados, tamanho_dados);
    }
    pacote->crc = calcula_crc(pacote);
}

void envia_pacote(int soquete, const char *nome_interface, Pacote *pacote) {
    struct ifreq ifr;
    struct sockaddr_ll endereco = {0};

    // Obtém o índice da interface
    strncpy(ifr.ifr_name, nome_interface, IFNAMSIZ - 1);
    if (ioctl(soquete, SIOCGIFINDEX, &ifr) < 0) {
        perror("Erro ao obter índice da interface");
        exit(EXIT_FAILURE);
    }
    int indice_interface = ifr.ifr_ifindex;

    // Configura o endereço
    endereco.sll_family = AF_PACKET;
    endereco.sll_protocol = htons(ETH_P_ALL);
    endereco.sll_ifindex = indice_interface;
    endereco.sll_halen = ETH_ALEN;

    // Envia o pacote
    ssize_t enviado = sendto(soquete, pacote, sizeof(Pacote), 0, (struct sockaddr *)&endereco, sizeof(endereco));
    if (enviado < 0) {
        perror("Erro ao enviar pacote");
        exit(EXIT_FAILURE);
    } else {
        printf("Pacote enviado com sucesso! (%ld bytes)\n", enviado);
        
    }
}

int main() {
    // int soquete;
    char nome_interface[] = INTERFACE;
    struct ifreq ifr;

    // Cria o socket RAW
    int soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (soquete == -1) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    // Configura a interface
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, nome_interface, IFNAMSIZ - 1);
    if (ioctl(soquete, SIOCGIFINDEX, &ifr) < 0) {
        perror("Erro ao obter índice da interface");
        close(soquete);
        exit(EXIT_FAILURE);
    }
    int indice_interface = ifr.ifr_ifindex;
    printf("Índice da interface '%s': %d\n", nome_interface, indice_interface);

    // Prepara o pacote
    Pacote pacote = {0};
    const char *nome_arquivo = "arquivo_teste.txt";
    monta_pacote(&pacote, CMD_BACKUP, 1, (unsigned char *)nome_arquivo, strlen(nome_arquivo) );

    // pacote.marcador = MARCADOR_INICIO;
    // pacote.tipo = CMD_VERIFICA;
    // pacote.sequencia = 0;

    // const char *nome_arquivo = "arquivo_teste.txt";
    // pacote.tamanho = strlen(nome_arquivo);   
    // memcpy(pacote.dados, nome_arquivo, pacote.tamanho);

    // pacote.crc = calcula_crc(&pacote);

    // Envia o pacote
    envia_pacote(soquete, nome_interface, &pacote);

    // Aguarda a resposta do servidor
    unsigned char buffer[sizeof(Pacote)];
    struct sockaddr_ll endereco_servidor;
    socklen_t endereco_len = sizeof(endereco_servidor);

    ssize_t recebido = recvfrom(soquete, buffer, sizeof(buffer), 0, (struct sockaddr *)&endereco_servidor, &endereco_len);
    if (recebido < 0) {
        perror("Erro ao receber resposta do servidor");
        close(soquete);
        exit(EXIT_FAILURE);
    }

    Pacote *resposta = (Pacote *)buffer;

    // Verifica o marcador
    if (resposta->marcador != MARCADOR_INICIO) {
        printf("Resposta inválida do servidor: marcador incorreto (%x)\n", resposta->marcador);
        close(soquete);
        exit(EXIT_FAILURE);
    }

    // Verifica o CRC
    unsigned char crc_recebido = resposta->crc;
    resposta->crc = 0; // Zera o CRC antes de recalcular
    if (calcula_crc(resposta) != crc_recebido) {
        printf("Resposta inválida do servidor: CRC incorreto\n");
        close(soquete);
        exit(EXIT_FAILURE);
    }

    // Verifica o tipo de resposta
    if (resposta->tipo == ACK) {
        printf("Servidor respondeu com ACK!\n");
    } else if (resposta->tipo == NACK) {
        printf("Servidor respondeu com NACK!\n");
    } else {
        printf("Resposta inesperada do servidor. Tipo: %d\n", resposta->tipo);
    }
    printf("Resposta recebida: Marcador=%x, Tipo=%d, Tamanho=%d, CRC=%x\n",
       resposta->marcador, resposta->tipo, resposta->tamanho, resposta->crc);


    // Fecha o socket
    close(soquete);
    return 0;
}
