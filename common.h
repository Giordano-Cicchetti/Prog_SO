//FC in order to avoid that this file is included twice generating a compilation error
#ifndef COMMON_H
#define COMMON_H

//FC macros for handling errors
#define handle_error_en(en, msg)    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error(msg)           do { perror(msg); exit(EXIT_FAILURE); } while (0)

//FC parameters of configuration 
#define DEBUG           0 // FC display debug messages
#define SERVER_ADDRESS  "127.0.0.1"
#define SERVER_COMMAND  "QUIT\n"
#define SERVER_PORT     2015

//GC defining headers for struct message
#define LOGIN               0 //sent from the client to the server to login
#define LOGIN_OK            1 //sent from the server to confirm login
#define LOGIN_KO            2 //sent from the server to deny login

#define PREREGISTRATION     3 //sent from the client with the credential chosen for the registration
#define PREREGISTRATION_OK  4 //sent from the server to confirm they are usable
#define PREREGISTRATION_KO  5 //sent from the server to deny preregistration

#define REGISTRATION        6 //sent from the client to ask for registration
#define REGISTRATION_OK     7 //sent from the server to confirm registration
#define REGISTRATION_KO     8 //sent from the server to deny registration

#define NORMAL_MESSAGE      9 //sent from the client as a message of the chat

//GC for the 2 ticks ( Whatsapp-like )
#define SERVER_RECV         10 //sent to the client when the server has received the message (9)
#define CLIENT_READ         11 //sent to the client when the other client of the chat is inside

#define CHAT_REQUEST        12 //sent from the client when the interlocutor is chosen 
#define CHAT_OK             13 //sent from the server when the chat is created
#define CHAT_KO             14 //sent from the client when asks to leave the chat 

#define USER_LIST_REQUEST   15 //sent from the client to ask for the users list
#define USER_LIST_RESPONSE  16 //sent from the server with the users list

//GC to compare Users, defined in "structures.c" as User_compare and passed in "server.c" to the NormalFileSearch operation
typedef int (*CompareFn)(void* v1, void* v2);

#endif
