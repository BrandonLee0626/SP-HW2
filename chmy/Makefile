# Makefile

CC      = gcc
CFLAGS  = -Wall -Wextra -O2

CLIENT_BIN = client

all: $(CLIENT_BIN)

$(CLIENT_BIN): client.c board.c cJSON.c board.h cJSON.h
	$(CC) $(CFLAGS) -o $(CLIENT_BIN) client.c board.c cJSON.c

clean:
	rm -f $(CLIENT_BIN)
