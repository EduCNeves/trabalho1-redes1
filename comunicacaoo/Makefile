CC = gcc
CFLAGS = -Wall -O2
TARGETS = client_raw server_raw

all: $(TARGETS)

client_raw: client_raw.c
	$(CC) $(CFLAGS) -o client_raw client_raw.c

server_raw: server_raw.c
	$(CC) $(CFLAGS) -o server_raw server_raw.c

clean:
	rm -f $(TARGETS)
