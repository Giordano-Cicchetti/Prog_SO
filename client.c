#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons() and inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>

#include "common.h"
#include "ANSI-color-codes.h"

// FC main
int main(int argc, char* argv[]) {

    // FC values returned by the syscalls called in the following part, bytes read and sent every time something is arrived
    int ret,bytes_sent,recv_bytes;

    // FC variables even initialized for handling the socket of the client and of the server
    int socket_desc;
    struct sockaddr_in server_addr = {0}; 

    /* FC create a socket for contacting the server using IPV4 and UDP protocol */
    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc < 0)
        handle_error("Could not create socket");

    // FC debugging
    if (DEBUG) fprintf(stderr, "Socket created...\n");

    /* FC set up parameters for the connection and initiate a connection to the server,
    we must specify the server address, family and port */
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS); // FC server address
    server_addr.sin_family      = AF_INET; // FC IPV4 addresses
    server_addr.sin_port        = htons(SERVER_PORT); // FC don't forget about network byte order! using htons() method

    // FC buffer filled with zeros and its length and size
    char buf[1024];
    size_t buf_len = sizeof(buf);
    int msg_len;
    memset(buf,0,buf_len);

    // FC main loop
    while (1) {

        // FC quit command and its size
        char* quit_command = SERVER_COMMAND;
        size_t quit_command_len = strlen(quit_command);

        // FC asking for the message to send
        printf("Insert your message: ");

        // FC read a line from stdin, fgets() reads up to sizeof(buf)-1 bytes and on success returns the buf passed as argument
        if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
            fprintf(stderr, "Error while reading from stdin, exiting...\n");
            exit(EXIT_FAILURE);
        }

        // FC length of the message
        msg_len = strlen(buf);

		// FC send message to server, sendto() with flags = 0 is equivalent to write() to a descriptor
        bytes_sent=0;
        while ( bytes_sent < msg_len) {
            ret = sendto(socket_desc, buf, msg_len, 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
            if (ret == -1 && errno == EINTR) continue;
            if (ret == -1) handle_error("Cannot write to the socket");
            bytes_sent = ret;
        }

        // FC debugging
        if (DEBUG) fprintf(stderr, "Sent message of %d bytes...\n", bytes_sent);


        // FC if the message is "QUIT\n", client shutdown exiting the loop
		if (msg_len == quit_command_len && !memcmp(buf, quit_command, quit_command_len)){

            if (DEBUG) fprintf(stderr, "Sent QUIT command ...\n");
            break;

        }
        memset(buf,0,buf_len);
        // FC receive message from server, recvfrom() with flags = 0 is equivalent to read() from a descriptor
	    recv_bytes = 0;
    	do {
            ret = recvfrom(socket_desc, buf, buf_len, 0, NULL, NULL);
            if (ret == -1 && errno == EINTR) continue;
            if (ret == -1) handle_error("Cannot read from the socket");
	        if (ret == 0) break;
	        recv_bytes = ret;

	    } while ( recv_bytes<=0 );

        // FC debugging
        if (DEBUG) fprintf(stderr, "Received answer of %d bytes...\n",recv_bytes);

        // FC the message from the server is arrived
        printf("Server response: %s\n", buf); 
    }


    // FC after the loop ends for a "QUIT\n", close the socket and release unused resources
    ret = close(socket_desc);
    if (ret < 0) handle_error("Cannot close the socket");

    // FC debugging
    if (DEBUG) fprintf(stderr, "Socket closed...\n");

    // FC debugging
    if (DEBUG) fprintf(stderr, "Exiting...\n");

    // FC exiting with success
    exit(EXIT_SUCCESS);
}
