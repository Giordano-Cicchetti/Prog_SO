// FC in order to avoid that this file is included twice generating a compilation error
#ifndef COMMON_H
#define COMMON_H

// FC macros for handling errors
#define handle_error_en(en, msg)    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error(msg)           do { perror(msg); exit(EXIT_FAILURE); } while (0)

// FC parameters of configuration 
#define DEBUG           0   // FC display debug messages
#define SERVER_ADDRESS  "127.0.0.1"
#define SERVER_COMMAND  "QUIT\n"
#define SERVER_PORT     2015

//define header for struct message
#define LOGIN               0
#define LOGIN_OK            1
#define LOGIN_KO            2 

#define PREREGISTRATION     3
#define PREREGISTRATION_OK  4
#define PREREGISTRATION_KO  5

#define REGISTRATION        6
#define REGISTRATION_OK     7
#define REGISTRATION_KO     8

#define NORMAL_MESSAGE      9

#define SERVER_RECV         10 //GC send to a client when the server has received the message
#define CLIENT_RECV         11 //send to 1st client when 2nd client signed_in
#define CLIENT_READ         12 //send to 1st client when 2nd client ask to communicate

#define CHAT_REQUEST        13
#define CHAT_OK             14
#define CHAT_KO             15

typedef int (*CompareFn)(void* v1, void* v2);

#endif
