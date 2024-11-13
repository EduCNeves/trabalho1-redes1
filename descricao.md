
# Trabalho de Redes: Sistema de Backup de Arquivos com RAWSockets

## Descrição do Trabalho

O objetivo deste trabalho é implementar um sistema de backup de arquivos utilizando RAWSockets. O sistema será composto por um **cliente** e um **servidor**, onde o cliente poderá enviar comandos para o servidor realizar operações de backup, restauração e verificação de arquivos. O sistema de comunicação entre os dois usará RAWSockets, e todos os dados serão transmitidos diretamente entre os dois pontos.

## Comandos Implementados

O sistema deve suportar os seguintes comandos:

1. **backup `<nome_arquivo>`**: Realiza o backup de um arquivo do cliente para o servidor.
2. **restaura `<nome_arquivo>`**: Restaura um arquivo do servidor para o cliente.
3. **verifica `<nome_arquivo>`**: Verifica se o arquivo no cliente e no servidor são iguais, utilizando o comando `cksum` para comparação de checksum.

### Códigos de Comando

| **Comando**  | **Código Tipo** | **Descrição**                                 |
|--------------|-----------------|-----------------------------------------------|
| **backup**   | 00100           | Envia um arquivo para o backup no servidor    |
| **restaura** | 00101           | Restaura um arquivo do servidor para o cliente|
| **verifica** | 00110           | Verifica se os arquivos do cliente e do servidor são iguais (cksum) |

### Códigos de Confirmação

| **Código Tipo** | **Descrição**                                 |
|-----------------|-----------------------------------------------|
| 00000           | ACK                                           |
| 00001           | NACK                                          |
| 00010           | OK                                            |
| 01101           | OK + CHECKSUM                                 |
| 01110           | OK + TAM                                      |
| 01111           | TAMANHO                                       |
| 10000           | DADOS                                         |
| 10001           | FIM DE DADOS                                  |
| 11111           | ERRO                                          |

## Detalhamento da Estrutura das Mensagens

As mensagens trocadas entre o cliente e o servidor seguem um formato específico, com um cabeçalho que contém informações sobre o tipo da mensagem, tamanho dos dados, sequência e verificação de integridade.

### Estrutura de uma Mensagem

A estrutura de uma mensagem é dividida da seguinte forma:

| **Intervalo de Bits** | **Campo**           | **Tamanho**  | **Descrição**                                |
|-----------------------|---------------------|--------------|----------------------------------------------|
| 0 - 7                 | Marcador de Início  | 1 byte       | Identificador de início da mensagem          |
| 8 - 13                | Tam                 | 6 bits       | Tamanho dos dados da mensagem                |
| 14 - 18               | Seq                 | 5 bits       | Número de sequência do pacote                |
| 19 - 23               | Tipo                | 5 bits       | Tipo de mensagem (comandos como backup, restaura, etc.) |
| 24 - 87               | Dados               | 0 a 63 bytes | Dados enviados (pode variar dependendo do comando) |
| 88 - 95               | CRC                 | 8 bits       | Cálculo de integridade (CRC de 8 bits)       |

## Requisitos Adicionais

1. **Fragmentação de Pacotes**: Como o RAWSocket não garante que os pacotes serão enviados ou recebidos inteiros, é necessário implementar um mecanismo de fragmentação e reassemblagem dos pacotes no cliente e no servidor.
   
2. **Tamanho Mínimo do Pacote**: Deve-se garantir que o pacote enviado tenha pelo menos 14 bytes, conforme as limitações de algumas placas de rede.

3. **Checksum**: O comando `verifica` deve comparar os checksums do arquivo no cliente e no servidor, utilizando o comando `cksum` para garantir que os arquivos sejam iguais.

4. **Timeout**: A comunicação deve respeitar um timeout para garantir que pacotes perdidos ou demorados sejam tratados corretamente.

5. **Relatório**: Um relatório detalhado deve ser entregue, contendo as escolhas de implementação feitas pelo grupo, bem como a documentação sobre o protocolo implementado.

## Explicação dos Arquivos

- **client.c**: Arquivo principal do cliente, onde o comando do usuário é lido e as funções correspondentes são chamadas. O cliente interage com o servidor por meio de comandos e recebe respostas.

- **commands.c**: Contém as funções para os comandos principais do cliente, como **backup**, **restaura** e **verifica**. Cada função implementa a lógica para interagir com o servidor, como enviar ou solicitar arquivos.

- **connection.c**: Lida com a comunicação entre o cliente e o servidor. Embora não envolva sockets no início, este arquivo prepara a estrutura de comunicação entre os dois, com funções como `connect_to_server()` e `send_command()`.

- **util.c**: Funções utilitárias como **validar_arquivo**, que verifica se um arquivo existe antes de tentar manipulá-lo. Também pode incluir funções para calcular o checksum e outras verificações.

- **client.h, commands.h, connection.h, util.h**: Arquivos de cabeçalho que definem as declarações das funções de cada módulo (cliente, comandos, comunicação e utilidades). Eles são incluídos nos respectivos arquivos de implementação.

- **Makefile**: Arquivo para automatizar a compilação do projeto. Ele especifica as dependências e os comandos de compilação para o cliente, comandos, utilitários, e outros módulos do projeto.

## Conclusão

Este trabalho visa implementar um sistema robusto de backup e restauração de arquivos em rede, simulando um ambiente real de transferência de dados usando RAWSockets. O protocolo de comunicação deve ser cuidadosamente implementado para lidar com as limitações da rede, como a fragmentação de pacotes e o tamanho mínimo dos pacotes.


## Conclusão

Este trabalho visa implementar um sistema robusto de backup e restauração de arquivos em rede, simulando um ambiente real de transferência de dados usando RAWSockets. O protocolo de comunicação deve ser cuidadosamente implementado para lidar com as limitações da rede, como a fragmentação de pacotes e o tamanho mínimo dos pacotes.