#FC makefile in order to simplify the compilation procedure: just run "make" command in the terminal and "make clean" to clear the executable files

CC = gcc
CFLAGS = -Wall -g

all: server 

server: server.c common.h structures.h binary_file_search.h structures.c
	$(CC) $(CFLAGS) -o server server.c binary_file_search.c structures.c


.PHONY: clean
clean:
	rm -f  server 
