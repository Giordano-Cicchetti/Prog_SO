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

#include "common.h"
#include "ANSI-color-codes.h"
#include "structures.h"
#include "binary_file_search.h"

#define MAX_NUM_ITEMS 5


// FC to test the functions created during the development
int main(int argc, char** argv) {
  ListHead list; 
  Add_message_to_list(list,3,"ITALIA","feffo","gio");
  Add_message_to_list(list,3,"INGHILTERRA","gio","feffo");
  Remove_all_messages_from_list(list);
  
}
