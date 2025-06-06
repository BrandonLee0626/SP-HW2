all: client server

client: client.c
	gcc client.c cjson/cJSON.c -I./cjson -o client

server: server.c
	gcc server.c game_logic/game_logic.c cjson/cJSON.c -I./cjson -o server

clean:
	rm -f client server
