# FC makefile in order to simplify the compilation procedure: just run "make" command in the terminal and "make clean" to clear the executable files
CC = gcc
CFLAGS = -Wall -g

all: server client

server: server.c common.h
	$(CC) $(CFLAGS) -o server server.c

client: client.c common.h
	$(CC) $(CFLAGS) -o client client.c

.PHONY: clean
clean:
	rm -f client server
