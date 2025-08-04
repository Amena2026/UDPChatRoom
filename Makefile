
CC = gcc
CFLAGS = -Wall

all: client/UDPClient server/UDPServer

client/UDPClient: client/UDPClient.c server/DieWithError.c
	$(CC) $(CFLAGS) -o client/UDPClient client/UDPClient.c server/DieWithError.c -lpthread

server/UDPServer: server/UDPServer.c server/DieWithError.c
	$(CC) $(CFLAGS) -o server/UDPServer server/UDPServer.c server/DieWithError.c

clean:
	rm -f client/UDPClient server/UDPServer
