all: client server board

client: client.c
	gcc client.c cjson/cJSON.c -I./cjson -o client

server: server.c
	gcc server.c game_logic/game_logic.c cjson/cJSON.c -I./cjson -o server

board board.c
	g++ -o board board.c -I./include -L. -lrgbmatrix -lrt -lm -lpthread

clean:
	rm -f client server
