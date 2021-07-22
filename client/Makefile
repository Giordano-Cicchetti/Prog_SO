#FC makefile in order to simplify the compilation procedure: just run "make" command in the terminal and "make clean" to clear the executable files

CC = gcc
CFLAGS = -Wall -g

all:  client 

client: client.c common.h structures.h  structures.c
	$(CC) $(CFLAGS) -pthread -o client client.c  structures.c

.PHONY: clean
clean:
	rm -f client  
