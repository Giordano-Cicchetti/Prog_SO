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
#include <pthread.h> // threads
#include <assert.h>
#include <fcntl.h>
#include <signal.h>

#include "common.h"
#include "structures.h"
#include "ANSI-color-codes.h"

//FC variables even initialized for handling the socket of the client and of the server
int socket_desc;
struct sockaddr_in server_addr = {0};

pthread_t receiver_thread;

//FC current user
char user[MAX_CREDENTIAL];

//FC handler for exit with SIGHUP or SIGINT/SIGTSTP (sigaction) or QUIT\n
void quit_handler(){
    Message m;
    Message* message;
    int ret;
     //GC close the receiver thread
    if(pthread_cancel(receiver_thread)) {
        printf("Error closing thread\n");
        exit(EXIT_FAILURE);
    }

    //FC sending CHAT_KO to the server with from so that server can delete him/her as a User Online
    Message_init(&m,CHAT_KO,user,NULL,NULL,0);
    int bytes_sent=0;
    while ( bytes_sent < MESSAGE_SIZE ) {
        ret = sendto(socket_desc, &m , MESSAGE_SIZE, 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) handle_error("Cannot write to the socket");
        bytes_sent = ret;
    }

    //FC exiting
    printf(BGRN "\n\nExiting...\n");

   

    //FC wait for response from the server with CHAT_KO otherwise it will restart the quit_handler
    char buf[MESSAGE_SIZE];
    memset(buf,0,MESSAGE_SIZE);
    int recv_bytes = 0;
    do {
        ret = recvfrom(socket_desc, buf, MESSAGE_SIZE, 0, NULL, NULL);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) handle_error("Cannot read from the socket");
        if (ret == 0) break;
        recv_bytes = ret;
        
    } while ( recv_bytes<=0 );

    message=(Message*)buf;
    int response=message->header;
    if(response != CHAT_KO) {printf("exit again"); quit_handler(); return;}

    ret = close(socket_desc);
    if (ret < 0) handle_error("Cannot close the socket");

    //FC debugging
    if (DEBUG) printf(BGRN "\n\nSocket closed...\n");

    //FC exiting with success
    exit(EXIT_SUCCESS);
}

//GC receiving thread : it must only print the messages (HEADER:9) received
void* receiver_handler(void *arg) {

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

        Message * m=(Message*)buf;

        //FC debugging
        if (DEBUG) printf("Received answer of %d bytes...\n",recv_bytes);
        
        if(m->header==SERVER_RECV){
            //Do something
            printf("\xE2\x9C\x93\n");
        }
        else if(m->header==CLIENT_READ){
           
            printf("\xE2\x9C\x93\n");
        }
        else if(m->header==CHAT_JOIN){
            printf("\n");
            printf(BYEL MOVE_CENTRE UYEL "%s has join the chat\e[1;32m" RESET BGRN, m->content); 
            printf("\n\n");
           
        }
        else if(m->header==CHAT_KO){
            printf("Server shutdown \n Exiting...\n");
            close(socket_desc);
            exit(EXIT_SUCCESS);
        }

        //FC the message from the server arrived is printed
        if(strcmp(user,m->from)==0){
           
            printf(BGRN "%s\e[1;32m\n", m->content);
        }
        else if(strcmp(user,m->to)==0){
            
            printf(BRED MOVE_RIGHT "%s\e[1;32m\n", m->content); 
        }

    }
    pthread_exit(NULL);
}


//FC main
int main(int argc, char* argv[]) {
    
  //FC install CTRL-C (SIGINT) and CTRL-Z (SIGTSTP) signal handler and kill terminal handler (SIGHUP)
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &quit_handler;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGTSTP , &action, NULL);

    //FC values returned by the syscalls called in the following part, bytes read and sent every time something is arrived
    int ret,bytes_sent,recv_bytes;  

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

    //GC create the struct user and a struct for message sent and one for that read
    User u;
    Message m;
    Message* message;

    //##############################1STPHASE#######################################################################################

    char* username = (char*)malloc(sizeof(char)*MAX_CREDENTIAL);
    char* password = (char*)malloc(sizeof(char)*MAX_CREDENTIAL);

    printf(BMAG "\nWelcome to the Private Chat!\n\n");
    while(1){

        memset(buf,0,buf_len);
        printf(BGRN "Have you registered yet?(Y/N): ");
        char c = (char) getchar();
        while(getchar()!='\n');

        //GC if the user want to sign_in
        if(c=='Y' || c=='y'){
            //GC take the username
            printf(BBLU "\nInsert your nickname:  ");
            scanf("%s",username);
            //GC take the password
            printf(BBLU "\nInsert your password:  ");
            scanf("%s",password);

            printf(BBLU "\nUsername:%s  Password:%s \n\n",username,password);
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
                printf(BRED "You are logged now! \n\n");
                //GC this clear the stdin
                while(getchar()!='\n');
                strcpy(user,username);
                break;
            }
            //GC else if username or password is incorrect or he/she is already online continue the loop
            else {
                
                if(strcmp(message->content, "login failed") == 0){
                    printf("Username o Password errata \n");
                }
                
                else{
                    printf("Username already online \n");
                }
              
                //GC this clear the stdin
                while(getchar()!='\n');
                continue;
            }
        }


        //GC if the user want to sign_up
        else if(c=='N' || c=='n'){
            //GC take the username
            printf("You have to register to continue...\nInsert a nickname:  ");
            scanf("%s",username);
            //GC take the password
            printf("\nInsert a password:  ");
            scanf("%s",password);
            printf("\nUsername: %s  Password: %s \n",username,password);
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
                printf("Username already used! \n");
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
                    printf("Error adding user \n");
                    return 1;
                }
                strcpy(user,username);
                //GC this clear the stdin
                while(getchar()!='\n');
                break;
            }
            //GC if user insert not a Y or a N continue the loop
        }
    }

    //GC free the helper buffers 
    free(username);
    free(password);

//#############################ENDOF1STPHASE#######################################################################################

//###############################2NDPHASE##########################################################################################

    //GC choose the interlocutor
    Message_init(&m,USER_LIST_REQUEST,NULL,NULL,NULL,0);
    bytes_sent=0;
    while ( bytes_sent < MESSAGE_SIZE) {
        ret = sendto(socket_desc, &m , MESSAGE_SIZE, 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) handle_error("Cannot write to the socket");
        bytes_sent = ret;
    }

    //GC wait for response from the server
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

    int response=message->header;
    char interlocutor[MAX_CREDENTIAL];
    if(response==USER_LIST_RESPONSE){
        char* list=message->content;
        Check_registered_user(list,interlocutor,user);
    }
    printf(BRED "\nYour interlocutor is: %s \n",interlocutor);
    if(strcmp(interlocutor,SERVER_COMMAND)==0) quit_handler(); //FC goto is ok since we have only one end-point


    //FC request for a chat between user and interlocutor choosen at the previous point
    Message_init(&m,CHAT_REQUEST,user,NULL,interlocutor,strlen(interlocutor));
    bytes_sent=0;
    while ( bytes_sent < MESSAGE_SIZE) {
        ret = sendto(socket_desc, &m , MESSAGE_SIZE, 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) handle_error("Cannot write to the socket");
        bytes_sent = ret;
    }

    //FC wait for response from the server CHAT_OK
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

    //FC case 1: Chat created by the server (CHAT_OK)
    if (response==CHAT_OK){
        //FC debugging
        if (DEBUG) fprintf(stderr, "The chat between you and %s has been successfully created in our server \n", interlocutor);
    }
    else{
    //FC case 2: EXIT
        quit_handler();
    }

//##############################ENDOF2NDPHASE#######################################################################################


//###############################3THDPHASE############################################################################################

    //GC create a thread to handle receiving messages and detach it
    
    ret = pthread_create(&receiver_thread, NULL, receiver_handler , (void *)&socket_desc);
    if (ret) handle_error_en(ret, "Could not create a new thread for receiving messages for your chat");
    
    if (DEBUG) fprintf(stderr, "(Thread created) Now you can receive messages from your chat!\n");
        
    ret = pthread_detach(receiver_thread); //GC we won't call phtread_join() on this thread
    if (ret) handle_error_en(ret, "Could not detach the receiving thread");
    
    //FC asking for the message to send
    printf(BGRN "\nYou just entered the chat with %s \n\n",interlocutor);
    if (DEBUG) fprintf(stderr, "(Thread created) Now you can send messages from your chat!\n");

    //FC main loop to handle sending thread: it must send messages (HEADER:9) to the server
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

        //FC if the message is "QUIT\n", client shutdown exiting the loop
		if (msg_len == quit_command_len && !memcmp(buf, quit_command, quit_command_len)){

            if (DEBUG) fprintf(stderr, "Sent QUIT command ...\n");
            quit_handler();

        }
        Message_init(&m,NORMAL_MESSAGE,user,interlocutor,(void*)buf,msg_len+1);
		//FC send message to server, sendto() with flags = 0 is equivalent to write() to a descriptor
        bytes_sent=0;
        while ( bytes_sent < msg_len) {
            ret = sendto(socket_desc, &m , MESSAGE_SIZE, 0, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
            if (ret == -1 && errno == EINTR) continue;
            if (ret == -1) handle_error("Cannot write to the socket");
            bytes_sent = ret;
        }

        //FC debugging
        if (DEBUG) fprintf(stderr, "Sent message of %d bytes...\n", bytes_sent);


        
        memset(buf,0,buf_len);
        
    }
    

//##############################ENDOF3THDPHASE############################################################################################

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
