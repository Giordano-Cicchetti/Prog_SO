#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>

#include "common.h"
#include "ANSI-color-codes.h"
#include "structures.h"
#include "binary_file_search.h"

//FC lists used by the server to handle users online and chats created during its lifetime
ListHead chat_list;
ListHead usersonline_list;

//GC file descriptor for users' file
int fd;
//GC number of users registered
int num_users;

//FC socket descriptor
int socket_desc;

//FC handler for exit with SIGHUP or SIGINT/SIGTSTP (sigaction)
void quit_handler(){


    //send CHAT_KO to all user_online
    // TODO ******
    //
    //
    //************


    //FC closing the file
    close(fd);
    
    int ret = close(socket_desc);
    if (ret < 0) handle_error("Cannot close the socket");

    //FC debugging
    if (DEBUG) printf(BGRN "\n\nSocket closed...\n");

    //FC exiting
    printf(BGRN "\n\nExiting...\n");

    //FC exiting with success
    exit(EXIT_SUCCESS);
}

//##############################SERVER_HANDLER############################################################################################

/*FC method for processing incoming requests, it takes as argument
 the socket descriptor for the incoming connection */
void* connection_handler(int socket_desc) {
    
    printf(BMAG "\nServer of the Private Chat!\n\n");
    //FC values returned by the syscalls called in the following part, bytes read and sent every time something is arrived
    int ret, recv_bytes, bytes_sent;

    //FC buffer 
    char buf[1024];
    
    //FC size of the buffer
    size_t buf_len = sizeof(buf);
   
    //FC setting buffer array with all zeros
    memset(buf,0,buf_len);

    //FC quit command and its size
    char* quit_command = SERVER_COMMAND;
    size_t quit_command_len = strlen(quit_command);

    //FC initialize the sockaddr_in structure of the client that is writing to the server
    struct sockaddr_in client_addr;

    //FC size of the struct sockaddr_in of the client
    int sockaddr_len = sizeof(client_addr); 

    //FC echo loop
    while (1) {

        //FC setting buffer array with all zeros
        memset(buf,0,buf_len);

        /*FC read message from clients, we do not deal 
           with partially sent messages since it is UDP protocol */
        
        //FC bytes received
        recv_bytes = 0;
        
        //FC receiving
        do {
            printf(BGRN "\nReceiving.. \n" reset);
            recv_bytes = recvfrom(socket_desc, buf, buf_len, 0, (struct sockaddr *) &client_addr, (socklen_t *) &sockaddr_len);
            if (recv_bytes == -1 && errno == EINTR) continue;
            if (recv_bytes == -1) handle_error("Cannot read from the socket");
            if (recv_bytes == 0) break;
            
		} while ( recv_bytes <= 0 );


        //FC setting the ip address to a struct sockaddr_in -> struct in_addr -> field "s_addr" of "in_addr_t" type (in NBO)
        //client_addr.sin_addr.s_addr = inet_addr("10.0.0.1");

        //FC taking the ip address from the clientaddr struct so that server can know who is the client
        char* dst=NULL;
        dst = inet_ntoa(client_addr.sin_addr); //FC return the IP
        printf(BRED "\nFrom %s \n" reset ,dst); //FC prints "10.0.0.1"
     

        //FC debugging
        if (DEBUG) {
                fprintf(stderr, "Received command of %d bytes...\n", recv_bytes);
        }

        //GC I see the incoming bytes as a message struct

        Message* message= (Message*)buf;
        Message m;

        //GC I check for the type of the message looking into the header
        int header = message->header;



        //------------DIFFERENT RESPONSES TO DIFFERENT MESSAGE HEADERS---------------


        //**HEADER 0-1-2** : LOGIN

        //GC if the message is a request of login do some stuff
        if(header==LOGIN){
            User* u = (User*) message->content;
            printf(BRED "\n%s " reset ,u->username);
            printf(BWHT "asks for login\n" reset);
            int pos=normalFileSearch(fd, u, sizeof(User), User_compare);
            
            //GC if user's credentials are right and that user IS NOT ONLINE send LOGIN_OK
            char* present=UserOnline_ispresent(&usersonline_list, u->username);
            if (pos >= 0 &&  present==NULL){
                Message_init(&m,LOGIN_OK,NULL,NULL,"login accept",13);
                printf(BRED "\n%s " reset ,u->username);
                printf(BWHT "Login accepted\n\n" reset);
            }
            //GC else send LOGIN_KO if is already online or if is not in the file of registered users
            else if (pos >= 0 && present != NULL){
                Message_init(&m,LOGIN_KO,NULL,NULL,"login failed (already online)",30);
                printf(BRED "%s Login denied \n" reset ,u->username);
            }
            else{ 
                Message_init(&m,LOGIN_KO,NULL,NULL,"login failed",13);
                printf(BRED "%s Login denied \n" reset ,u->username);

            }

            bytes_sent=0;
            while ( bytes_sent < MESSAGE_SIZE) {
                ret = sendto(socket_desc, &m, MESSAGE_SIZE, 0, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in));
                if (ret == -1 && errno == EINTR) continue;
                if (ret == -1) handle_error("Cannot write to the socket");
                bytes_sent = ret;
            }
            printf(BWHT "Sent message to the client for login \n");
            if(DEBUG)User_print(fd,num_users);
            continue;
        }
        
        
        //**HEADER 3-4-5** : PREREGISTRATION

        //GC if the message is a request of username validation do some other stuff
        else if(header==PREREGISTRATION){
            User* u = (User*) message->content;
            printf(BRED "Server: %s asks for valid username \n" reset ,u->username);
            //GC search if username already exists 
            int pos=normalFileSearch(fd, u, sizeof(User), User_compare_only_username);
            //GC if username exists send PREREGISTRATION_KO
            if(pos >= 0){
                Message_init(&m,PREREGISTRATION_KO,NULL,NULL,"username already exists",24);
                printf(BRED "Username already exists \n" reset);
                
            }
            //GC else send PREREGISTRATION_OK
            else{
                Message_init(&m,PREREGISTRATION_OK,NULL,NULL,"valid username",15);
                printf(BRED "Valid username \n" reset );
            }
            bytes_sent=0;
            while ( bytes_sent < MESSAGE_SIZE) {
                ret = sendto(socket_desc, &m, MESSAGE_SIZE, 0, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in));
                if (ret == -1 && errno == EINTR) continue;
                if (ret == -1) handle_error("Cannot write to the socket");
                bytes_sent = ret;
            }
            printf("sent message to the client for username validation \n");
            continue;

        }

        //**HEADER 6-7-8** : REGISTRATION

        //GC if the message is a request of registration do some other stuff
        else if(header==REGISTRATION){
            User* u = (User*) message->content;
            printf(BRED "Server: %s ask for registration \n" reset ,u->username);
            //GC write the user in the file to keep memory
            int res= binaryFileWrite(fd, u , USER_SIZE, num_users);
            //GC if not ok send REGISTRATION_KO
            if(res==-1){
                Message_init(&m,REGISTRATION_KO,NULL,NULL,"error in registration",22);
                printf(BRED "Error in registration \n" reset );
            }
            //GC else send REGISTRATION_OK
            else{
                Message_init(&m,REGISTRATION_OK,NULL,NULL,"registration ok",16);
                printf(BRED "Server: %s registration ok \n" reset ,u->username);
                num_users++;
            }
            bytes_sent=0;
            while ( bytes_sent < MESSAGE_SIZE) {
                ret = sendto(socket_desc, &m, MESSAGE_SIZE, 0, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in));
                if (ret == -1 && errno == EINTR) continue;
                if (ret == -1) handle_error("Cannot write to the socket");
                bytes_sent = ret;
            }
            printf("sent message to the client for registration \n");
            if(DEBUG)User_print(fd,num_users);
            continue;

        }

        //**HEADER 15-16** : REQUEST OF USERS LIST

        else if(header==USER_LIST_REQUEST){
            char dummy[num_users*MAX_CREDENTIAL];
            User_all_usernames(fd,dummy,num_users);
            Message_init(&m,USER_LIST_RESPONSE,NULL,NULL,(void*)dummy,sizeof(dummy));
            bytes_sent=0;
            while ( bytes_sent < MESSAGE_SIZE) {
                ret = sendto(socket_desc, &m, MESSAGE_SIZE, 0, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in));
                if (ret == -1 && errno == EINTR) continue;
                if (ret == -1) handle_error("Cannot write to the socket");
                bytes_sent = ret;
            }

            printf(BWHT "\nA client is asking for the users' list\n" reset);
            printf(BWHT "\nSent the user's list to the client\n");

            continue;
        }
        
        //**HEADER 12-13** : CHAT REQUEST - CHAT_OK
        else if(header==CHAT_REQUEST){
            
            char* user=message->from;
            char* interlocutor=message->content;
            printf(BRED "\nChat request from %s with %s\n",user,interlocutor);
            
            Chat* chat= NULL;
            chat=Chat_ispresent_between_users(&chat_list,user,interlocutor);

            //FC taking the ip address from the clientaddr struct so that server can know who is the client
            char* ip=NULL;
            ip = inet_ntoa(client_addr.sin_addr); //FC return the IP
        

            //FC chat is present: sending the previous message
            if (chat!=NULL){

                printf("\nChat already existing..\n");
                //FC creating the user online for user : if he/she is already online would not be here (login failed)
                Add_useronline_to_list(&usersonline_list,chat,user, ip,client_addr.sin_port);

                //FC printing useronline list and their chat
                UserOnline_list_print(&usersonline_list);
                Chat_print(chat);

                Message_init(&m,CHAT_OK,NULL,NULL,NULL,0);
                bytes_sent=0;
                while ( bytes_sent < MESSAGE_SIZE) {
                     ret = sendto(socket_desc, &m, MESSAGE_SIZE, 0, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in));
                     if (ret == -1 && errno == EINTR) continue;
                     if (ret == -1) handle_error("Cannot write to the socket");
                     bytes_sent = ret;
                }

                //GC send all messages in the list_msg to user
                MessageList_send(chat->list_msg,socket_desc,&client_addr);
            
                continue;
            }

            else{ //FC chat is NOT present

                printf("\nCreating chat..\n");
                ListHead* list_msg=(ListHead*)malloc(sizeof(ListHead)); 
                List_init(list_msg);
                Add_chat_to_list(&chat_list, user, interlocutor, list_msg); 

                Chat* user_chat=Find_chat_by_username(&chat_list, user);

                //FC creating the user online for user : if he/she is already online would not be here (login failed)
                Add_useronline_to_list(&usersonline_list, user_chat, user, ip,client_addr.sin_port);
                
                 //FC printing useronline list and all chats
                UserOnline_list_print(&usersonline_list);
                
                Chat_list_print(&chat_list);

                Message_init(&m,CHAT_OK,NULL,NULL,NULL,0);
                bytes_sent=0;
                while ( bytes_sent < MESSAGE_SIZE) {
                     ret = sendto(socket_desc, &m, MESSAGE_SIZE, 0, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in));
                     if (ret == -1 && errno == EINTR) continue;
                     if (ret == -1) handle_error("Cannot write to the socket");
                     bytes_sent = ret;
                }
                
                continue;
            }

        }

        //**HEADER 14** : CHAT_KO
        else if(header==CHAT_KO){
            //FC is CHAT_KO arrives the server remove the user as user online and send CHAT_KO back as a confirm
            char* from=message->from;
            UserOnline_list_print(&usersonline_list);
            Remove_useronline_from_list(&usersonline_list,from);
            printf("%s is asking to exit the Chat, the updated list of online users is..\n\n",from);
            UserOnline_list_print(&usersonline_list);
            
            Message_init(&m,CHAT_KO,NULL,NULL,NULL,0);
                bytes_sent=0;
                while ( bytes_sent < MESSAGE_SIZE) {
                     ret = sendto(socket_desc, &m, MESSAGE_SIZE, 0, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in));
                     if (ret == -1 && errno == EINTR) continue;
                     if (ret == -1) handle_error("Cannot write to the socket");
                     bytes_sent = ret;
                }
            printf("Sent to the client the confirm to exit");
            continue;

        }
        
        
        //**HEADER 9 NORMAL MESSAGE
        else if(header==NORMAL_MESSAGE){
            printf("ho ricevuto da %s \n",message->from);
            printf("%s\n",message->content);
            printf("questo client ha prota %d \n",client_addr.sin_port);

            Chat* c = Chat_ispresent_between_users(&chat_list,message->from,message->to);
            if(c==NULL){
                //error da gestire
                //TODO 
            }

            Add_message_to_list(c->list_msg,message->header,message->content,message->from,message->to);
            char* pippo=UserOnline_ispresent(&usersonline_list,message->to);//user to is online, send the message)
            if(pippo!=NULL && c==Give_useronline_Chat(&usersonline_list,message->to)){
                char* receiver_ip=Give_useronline_IP(&usersonline_list,message->to);
                client_addr.sin_addr.s_addr = inet_addr(receiver_ip);
                client_addr.sin_port=Give_useronline_Port(&usersonline_list,message->to);
                bytes_sent=0;
                while ( bytes_sent < MESSAGE_SIZE) {
                    ret = sendto(socket_desc, message , MESSAGE_SIZE, 0, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in));
                    if (ret == -1 && errno == EINTR) continue;
                    if (ret == -1) handle_error("Cannot write to the socket");
                    bytes_sent = ret;
                    }
                
            }
            continue;
        }

        // OTHER HEADERS!! ::::::::TODO:::::::::



//------------------------------------------RESPONSE TEMPORARY--------------------------------------------------------

        //FC receive message from client and print it as green bold text
        printf(BRED "Client: %s \n" reset ,buf);

        /*FC only if the bytes received are equal to the length of the "quit" command the comparison is made,
        in that case no sending is needed so we can restart the loop */
        if (recv_bytes == quit_command_len && !memcmp(buf, quit_command, quit_command_len)){

            if (DEBUG) fprintf(stderr, "Received QUIT command...\n");
            continue;

         }
        printf(" header %d \n from %s to %s \n",((Message*)buf)->header,((Message*)buf)->from,((Message*)buf)->to);
        memset(buf,0,buf_len);
        fprintf(stderr, BGRN  "Server:");
        //FC read a line from stdin, fgets() reads up to sizeof(buf)-1 bytes and on success returns the buf passed as argument
        if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
            fprintf(stderr, "Error while reading from stdin, exiting...\n");
            exit(EXIT_FAILURE);
        }

        printf("%s\n",buf);
         //FC length of the message
        int msg_len = strlen(buf);

		//FC send message to server, sendto() with flags = 0 is equivalent to write() with a descriptor
        bytes_sent=0;
        while ( bytes_sent < msg_len) {
            ret = sendto(socket_desc, buf, msg_len, 0, (struct sockaddr*) &client_addr, sizeof(struct sockaddr_in));
            if (ret == -1 && errno == EINTR) continue;
            if (ret == -1) handle_error("Cannot write to the socket");
            bytes_sent = ret;
        }

    }// end of while(1)

    //FC after the loop ends (now it is never), close the socket and release unused resources
    ret = close(socket_desc);
    if (ret < 0) handle_error("Cannot close socket for incoming connection");

    //FC debugging
    if (DEBUG) fprintf(stderr, "Socket closed...\n");

    //FC end of the handler function
    return NULL;

}

//##############################ENDOF_SERVER_HANDLER#############################################################
    
//####################################MAIN#############################################################################

//FC main
int main(int argc, char* argv[]) {

    //GC open the binary file with the users inside 
    fd=open(FILENAME, O_CREAT|O_RDWR,0666);

    //GC we get the size of the file
    struct stat stats;
    fstat(fd, &stats);

    //GC from the size, we determine the number of records
    int size=stats.st_size;
    num_users=size/USER_SIZE;
    assert(!(size%USER_SIZE));
    
    //FC lists initialized
    List_init(&chat_list);
    List_init(&usersonline_list);

    //FC values returned by the syscalls called in the following part
    int ret;

    //FC install CTRL-C (SIGINT) and CTRL-Z (SIGTSTP) signal handler and kill terminal handler (SIGHUP)
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = &quit_handler;
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGHUP, &action, NULL);
    sigaction(SIGTSTP, &action, NULL);

    //FC initialize the sockaddr_in structure of the server
    struct sockaddr_in server_addr = {0};

    //FC size of the struct sockaddr_in
    int sockaddr_len = sizeof(struct sockaddr_in); 

    /*FC create a socket for listening using AF_INET (IPV4) protocol (network layer) 
    and SOCK_DGRAM type for UDP protocol (transport layer) */
    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc < 0)
        handle_error("Could not create socket");

    //FC debugging
    if (DEBUG) fprintf(stderr, "Socket created...\n");

    /*FC we enable SO_REUSEADDR to quickly restart our server after a crash:
    for a temporary binding of the address in "bind" call*/
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    if (ret < 0)
        handle_error("Cannot set SO_REUSEADDR option");

    //FC set server address and bind it to the socket
    server_addr.sin_addr.s_addr = INADDR_ANY; // FC we want to accept connections from any interface
    server_addr.sin_family      = AF_INET; // FC IPV4 addresses
    server_addr.sin_port        = htons(SERVER_PORT); // FC don't forget about network byte order! using htons() method

    //FC bind address to socket
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sockaddr_len);
    if (ret < 0)
        handle_error("Cannot bind address to socket");

    //FC debugging
    if (DEBUG) fprintf(stderr, "Binded address to socket...\n");

    //FC loop to handle incoming connections (sequentially)
    while (1) {
        
		//FC ! remember : it is not required to accept an incoming connection in UDP protocol

        //FC debugging
        if (DEBUG) fprintf(stderr, "Opening connection handler...\n");

        //FC handler of each connection
        connection_handler(socket_desc);

    }

    //FC this will never be executed
    close(fd);
    exit(EXIT_SUCCESS); 
}

//####################################ENDOFMAIN#############################################################################