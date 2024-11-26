CC = gcc
CFLAGS = -Wall -g
SRC = main.c commands.c connection.c util.c packet.c
OBJ = $(SRC:.c=.o)
EXEC = cliente


$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
