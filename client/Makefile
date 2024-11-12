CC = gcc
CFLAGS = -Wall -g
SRC = client.c commands.c connection.c util.c
OBJ = $(SRC:.c=.o)
EXEC = cliente

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
