// FC in order to avoid that this file is included twice generating a compilation error
#ifndef COMMON_H
#define COMMON_H

// FC macros for handling errors
#define handle_error_en(en, msg)    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error(msg)           do { perror(msg); exit(EXIT_FAILURE); } while (0)

// FC parameters of configuration 
#define DEBUG           1   // FC display debug messages
#define SERVER_ADDRESS  "127.0.0.1"
#define SERVER_COMMAND  "QUIT\n"
#define SERVER_PORT     2015

#endif
