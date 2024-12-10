# Variáveis
CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = ../src
BUILD_DIR = ../build
TARGET = servidor

# Diretórios de fontes
COMMANDS_DIR = $(SRC_DIR)/commands
CONNECTION_DIR = $(SRC_DIR)/connection
PACKET_DIR = $(SRC_DIR)/packet
UTIL_DIR = $(SRC_DIR)/util

# Arquivos fonte
SOURCES = main.c \
          $(SRC_DIR)/commands.c \
          $(SRC_DIR)/connection.c \
          $(SRC_DIR)/packet.c \
          $(SRC_DIR)/util.c

# Objetos gerados
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Diretórios para incluir
INCLUDES = -I$(SRC_DIR)/commands -I$(SRC_DIR)/connection -I$(SRC_DIR)/packet -I$(SRC_DIR)/util

# Regra padrão
all: $(BUILD_DIR) $(TARGET)

# Criação do executável
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

# Regra para compilar objetos
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Regra para criar o diretório de build
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Limpeza
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
