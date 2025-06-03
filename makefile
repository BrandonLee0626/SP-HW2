all: client server board temp

client: client.c
	gcc client.c cjson/cJSON.c -I./cjson -o client

server: server.c
	gcc server.c game_logic/game_logic.c cjson/cJSON.c -I./cjson -o server

board: board.c
	g++ -o board \
	board.c \
	rpi-rgb-led-matrix/lib/led-matrix-c.cc \
	rpi-rgb-matrix/lib/librgbmatrix.a \
	-I rpi-rgb-matrix/include \
	-lpthread -lm -lrt

temp: temp.c
	g++ -o temp \
	temp.c board.c \
	rpi-rgb-led-matrix/lib/led-matrix-c.cc \
	rpi-rgb-led-matrix/lib/librgbmatrix.a \
	-I rpi-rgb-led-matrix/include \
	-lpthread -lm -lrt

clean:
	rm -f client server board temp
