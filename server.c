#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>

#include "common.h"
#include "ANSI-color-codes.h"

/* FC method for processing incoming requests, it takes as argument
 the socket descriptor for the incoming connection */
void* connection_handler(int socket_desc) {

    // FC values returned by the syscalls called in the following part, bytes read and sent every time something is arrived
    int ret, recv_bytes, bytes_sent;

    // FC buffer 
    char buf[1024];
    
    // FC size of the buffer
    size_t buf_len = sizeof(buf);
   
    // FC setting buffer array with all zeros
    memset(buf,0,buf_len);

    // FC quit command and its size
    char* quit_command = SERVER_COMMAND;
    size_t quit_command_len = strlen(quit_command);

    // FC initialize the sockaddr_in structure of the client that is writing to the server
    struct sockaddr_in client_addr;

    // FC size of the struct sockaddr_in of the client
    int sockaddr_len = sizeof(client_addr); 

    // FC echo loop
    while (1) {

        // FC setting buffer array with all zeros
        memset(buf,0,buf_len);

        /* FC read message from clients, we do not deal 
           with partially sent messages since it is UDP protocol */
        
        // FC bytes received
        recv_bytes = 0;
        
        // FC receiving
        do {
             printf(BGRN "Receiving.. \n" reset);
            recv_bytes = recvfrom(socket_desc, buf, buf_len, 0, (struct sockaddr *) &client_addr, (socklen_t *) &sockaddr_len);
            if (recv_bytes == -1 && errno == EINTR) continue;
            if (recv_bytes == -1) handle_error("Cannot read from the socket");
            if (recv_bytes == 0) break;

		} while ( recv_bytes <= 0 );

        // FC debugging
        if (DEBUG) {
            
            fprintf(stderr, "Received command of %d bytes...\n", recv_bytes);
            
        }

        // FC receive message from client and print it as green bold text
        printf(BRED "Client: %s \n" reset ,buf);

        /* FC only if the bytes received are equal to the length of the "quit" command the comparison is made,
        in that case no sending is needed so we can restart the loop */
        if (recv_bytes == quit_command_len && !memcmp(buf, quit_command, quit_command_len)){

            if (DEBUG) fprintf(stderr, "Received QUIT command...\n");
            continue;

         }

        memset(buf,0,buf_len);
         fprintf(stderr, BGRN  "Server:");
        // FC read a line from stdin, fgets() reads up to sizeof(buf)-1 bytes and on success returns the buf passed as argument
        if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
            fprintf(stderr, "Error while reading from stdin, exiting...\n");
            exit(EXIT_FAILURE);
        }

        printf("%s\n",buf);
         // FC length of the message
        int msg_len = strlen(buf);

		// FC send message to server, sendto() with flags = 0 is equivalent to write() to a descriptor
        bytes_sent=0;
        while ( bytes_sent < msg_len) {
            ret = sendto(socket_desc, buf, msg_len, 0, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in));
            if (ret == -1 && errno == EINTR) continue;
            if (ret == -1) handle_error("Cannot write to the socket");
            bytes_sent += ret;
        }

    }

    // FC after the loop ends (now it is never), close the socket and release unused resources
    ret = close(socket_desc);
    if (ret < 0) handle_error("Cannot close socket for incoming connection");

    // FC debugging
    if (DEBUG) fprintf(stderr, "Socket closed...\n");

    // FC end of the handler function
    return NULL;

}


/* ########################################################################################################################## */
    

// FC main
int main(int argc, char* argv[]) {

    // FC values returned by the syscalls called in the following part
    int ret;

    // FC socket descriptor
    int socket_desc;

    // FC initialize the sockaddr_in structure of the server
    struct sockaddr_in server_addr = {0};

    // FC size of the struct sockaddr_in
    int sockaddr_len = sizeof(struct sockaddr_in); 

    /* FC create a socket for listening using AF_INET (IPV4) protocol (network layer) 
    and SOCK_DGRAM type for UDP protocol (transport layer) */
    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc < 0)
        handle_error("Could not create socket");

    // FC debugging
    if (DEBUG) fprintf(stderr, "Socket created...\n");

    /* FC we enable SO_REUSEADDR to quickly restart our server after a crash:
    for a temporary binding of the address in "bind" call*/
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    if (ret < 0)
        handle_error("Cannot set SO_REUSEADDR option");

    // FC set server address and bind it to the socket
    server_addr.sin_addr.s_addr = INADDR_ANY; // FC we want to accept connections from any interface
    server_addr.sin_family      = AF_INET; // FC IPV4 addresses
    server_addr.sin_port        = htons(SERVER_PORT); // FC don't forget about network byte order! using htons() method

    // FC bind address to socket
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sockaddr_len);
    if (ret < 0)
        handle_error("Cannot bind address to socket");

    // FC debugging
    if (DEBUG) fprintf(stderr, "Binded address to socket...\n");

    // FC loop to handle incoming connections (sequentially)
    while (1) {
        
		// FC ! remember : it is not required to accept an incoming connection in UDP protocol

        // FC debugging
        if (DEBUG) fprintf(stderr, "Opening connection handler...\n");

        // FC handler of each connection
        connection_handler(socket_desc);

    }

    // FC this will never be executed
    exit(EXIT_SUCCESS); 
}
