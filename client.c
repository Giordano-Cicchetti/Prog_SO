#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons() and inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <assert.h>
#include <fcntl.h>
#include "common.h"
#include "structures.h"
#include "ANSI-color-codes.h"
#include <pthread.h>

//GC receiving thread 
void * receiver_handler(void *arg) {

    int socket_desc = *((int*) arg);
    
    char buf[MESSAGE_SIZE];
    size_t buf_len = sizeof(buf);
    int recv_bytes;
    int ret;
    

    while(1){
        memset(buf,0,buf_len);
        //FC receive message from server, recvfrom() with flags = 0 is equivalent to read() from a descriptor
	    recv_bytes = 0;
    	do {
            ret = recvfrom(socket_desc, buf, buf_len, 0, NULL, NULL);
            if (ret == -1 && errno == EINTR) continue;
            if (ret == -1) handle_error("Cannot read from the socket");
	        if (ret == 0) break;
	        recv_bytes = ret;

	    } while ( recv_bytes<=0 );
        buf[recv_bytes-1]='\0';
        //FC debugging
        if (DEBUG) fprintf(stderr, "Received answer of %d bytes...\n",recv_bytes);

        //FC the message from the server is arrived
        printf(BRED MOVE_RIGHT "%s\e[1;32m\n", buf); 

    

    }
    pthread_exit(NULL);
}


//FC main
int main(int argc, char* argv[]) {

    //FC values returned by the syscalls called in the following part, bytes read and sent every time something is arrived
    int ret,bytes_sent,recv_bytes;

    //FC variables even initialized for handling the socket of the client and of the server
    int socket_desc;
    struct sockaddr_in server_addr = {0}; 

    /*FC create a socket for contacting the server using IPV4 and UDP protocol */
    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc < 0)
        handle_error("Could not create socket");

    //FC debugging
    if (DEBUG) fprintf(stderr, "Socket created...\n");

    /*FC set up parameters for the connection and initiate a connection to the server,
    we must specify the server address, family and port */
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS); // FC server address
    server_addr.sin_family      = AF_INET; // FC IPV4 addresses
    server_addr.sin_port        = htons(SERVER_PORT); // FC don't forget about network byte order! using htons() method

    //FC buffer for incoming messages filled with zeros and its length and size
    char buf[MESSAGE_SIZE];
    size_t buf_len = sizeof(buf);
    int msg_len;
    memset(buf,0,buf_len);


    //GC create the struct user and ask to the user if he want to sign_in or sign_up
    User u;
    Message m;
    Message* message;
    
    char* username = (char*)malloc(sizeof(char)*MAX_CREDENTIAL);
    char* password = (char*)malloc(sizeof(char)*MAX_CREDENTIAL);
    
    printf("Benvenuto nel programma di prova degli utenti! \n");
    while(1){

        memset(buf,0,buf_len);
        printf("Sei già registrato?(Y/N) ");
        char c = (char) getchar();
        while(getchar()!='\n');

        //GC if the user want to sign_in
        if(c=='Y' || c=='y'){
            //GC take the username
            printf("\ninserisci il tuo nickname:  ");
            scanf("%s",username);
            //GC take the password
            printf("\ninserisci la tua password:  ");
            scanf("%s",password);

            printf("\nUsername:%s  Password:%s \n",username,password);
            //GC initialize the user struct
            User_init(&u, username , password );

            //GC inizializza struttura messaggio di login e invia 
            //chiedendo al server di convalidare le credenziali
            Message_init(&m,LOGIN,NULL,NULL,(void*)&u,USER_SIZE);
            bytes_sent=0;
            while ( bytes_sent < MESSAGE_SIZE) {
                ret = sendto(socket_desc, &m , MESSAGE_SIZE, 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
                if (ret == -1 && errno == EINTR) continue;
                if (ret == -1) handle_error("Cannot write to the socket");
                bytes_sent = ret;
            }
            //FC debugging
            if (DEBUG) fprintf(stderr, "Sent message of %d bytes...\n", bytes_sent);

            //GC wait for server's response
            recv_bytes = 0;
    	    do {
                ret = recvfrom(socket_desc, buf, buf_len, 0, NULL, NULL);
                if (ret == -1 && errno == EINTR) continue;
                if (ret == -1) handle_error("Cannot read from the socket");
                if (ret == 0) break;
                recv_bytes = ret;

            } while ( recv_bytes<=0 );

            //GC the inner buffer is a message from the server
            message=(Message*)buf;

            int response=message->header;

            //GC if response is LOGIN_OK then quit the loop and go next
            if(response==LOGIN_OK) {
                printf("sei un utente loggato \n");
                //GC this clear the stdin
                while(getchar()!='\n');
                break;
            }
            //GC else if username or password is incorrect continue the loop
            else {
                printf("username o password errata \n");
                //GC this clear the stdin
                while(getchar()!='\n');
                continue;
            }
        }


        //GC if the user want to sign_up
        else if(c=='N' || c=='n'){
            //GC take the username
            printf("Per continuare bisogna registrarsi.\nInserire un nickname:  ");
            scanf("%s",username);
            //GC take the password
            printf("\ninserisci una password:  ");
            scanf("%s",password);
            printf("\nUsername:%s  Password:%s \n",username,password);
            //GC initialize the user struct
            User_init(&u, username , password );
            
            //GC inizializza struttura messaggio per chiedere se username già esistente e invia 
            Message_init(&m,PREREGISTRATION,NULL,NULL,(void*)&u,USER_SIZE);
            bytes_sent=0;
            while ( bytes_sent < MESSAGE_SIZE) {
                ret = sendto(socket_desc, &m , MESSAGE_SIZE, 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
                if (ret == -1 && errno == EINTR) continue;
                if (ret == -1) handle_error("Cannot write to the socket");
                bytes_sent = ret;
            }

            //FC debugging
            if (DEBUG) fprintf(stderr, "Sent message of %d bytes...\n", bytes_sent);

            //GC wait for server's response
            recv_bytes = 0;
    	    do {
                ret = recvfrom(socket_desc, buf, buf_len, 0, NULL, NULL);
                if (ret == -1 && errno == EINTR) continue;
                if (ret == -1) handle_error("Cannot read from the socket");
                if (ret == 0) break;
                recv_bytes = ret;

            } while ( recv_bytes<=0 );
            //GC the inner buffer is a message from the server
            message=(Message*)buf;
            int response=message->header;
            
            //GC if exists a user with this username continue the loop 
            if(response==PREREGISTRATION_KO) {
                printf("Username già in uso! \n");
                //GC this clear the stdin
                while(getchar()!='\n');
                continue;
            }
            //GC else ask the server to registrate the user, quit the loop and go next
            else {
                //GC inizializza struttura messaggio e invia 
                Message_init(&m,REGISTRATION,NULL,NULL,(void*)&u,USER_SIZE);
                bytes_sent=0;
                while ( bytes_sent < MESSAGE_SIZE) {
                    ret = sendto(socket_desc, &m , MESSAGE_SIZE, 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
                    if (ret == -1 && errno == EINTR) continue;
                    if (ret == -1) handle_error("Cannot write to the socket");
                    bytes_sent = ret;
                }
                //FC debugging
                if (DEBUG) fprintf(stderr, "Sent message of %d bytes...\n", bytes_sent);


                //GC Wait for response from the server
                memset(buf,0,buf_len);
                recv_bytes = 0;
                do {
                    ret = recvfrom(socket_desc, buf, buf_len, 0, NULL, NULL);
                    if (ret == -1 && errno == EINTR) continue;
                    if (ret == -1) handle_error("Cannot read from the socket");
                    if (ret == 0) break;
                    recv_bytes = ret;

                } while ( recv_bytes<=0 );

                message=(Message*)buf;

                response=message->header;

                //GC if response is equal to RESPONSE_KO, something went wrong so 
                //its better to quit the program
                if(response==REGISTRATION_KO){
                    printf("errore aggiunta user \n");
                    return 1;
                }
                
                //GC this clear the stdin
                while(getchar()!='\n');
                break;
            }
            //GC if user insert not a Y or a N continue the loop
        }
    }

    

    free(username);
    free(password);
    

    //GC create a thread to handler receiving messages
    pthread_t receiver_thread;
    ret = pthread_create(&receiver_thread, NULL, receiver_handler , (void *)&socket_desc);
    
    if (ret) handle_error_en(ret, "Could not create a new thread");
    
    if (DEBUG) fprintf(stderr, "New thread created to handle the request!\n");
        
    ret = pthread_detach(receiver_thread); //GC I won't phtread_join() on this thread
    if (ret) handle_error_en(ret, "Could not detach the thread");
    //FC asking for the message to send
    printf(BGRN "Start your chat:\n");

    //FC main loop to handler sending messages
    while (1) {

        //FC quit command and its size
        char* quit_command = SERVER_COMMAND;
        size_t quit_command_len = strlen(quit_command);

        
        
        
        //FC read a line from stdin, fgets() reads up to sizeof(buf)-1 bytes and on success returns the buf passed as argument
        if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
            fprintf(stderr, "Error while reading from stdin, exiting...\n");
            exit(EXIT_FAILURE);
        }

        //FC length of the message
        msg_len = strlen(buf);

		//FC send message to server, sendto() with flags = 0 is equivalent to write() to a descriptor
        bytes_sent=0;
        while ( bytes_sent < msg_len) {
            ret = sendto(socket_desc, buf, msg_len, 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
            if (ret == -1 && errno == EINTR) continue;
            if (ret == -1) handle_error("Cannot write to the socket");
            bytes_sent = ret;
        }

        //FC debugging
        if (DEBUG) fprintf(stderr, "Sent message of %d bytes...\n", bytes_sent);


        //FC if the message is "QUIT\n", client shutdown exiting the loop
		if (msg_len == quit_command_len && !memcmp(buf, quit_command, quit_command_len)){

            if (DEBUG) fprintf(stderr, "Sent QUIT command ...\n");
            break;

        }
        memset(buf,0,buf_len);
        
    }


    //FC after the loop ends for a "QUIT\n", close the socket and release unused resources
    ret = close(socket_desc);
    if (ret < 0) handle_error("Cannot close the socket");

    //FC debugging
    if (DEBUG) fprintf(stderr, "Socket closed...\n");

    //FC debugging
    if (DEBUG) fprintf(stderr, "Exiting...\n");

    //FC exiting with success
    exit(EXIT_SUCCESS);
}
