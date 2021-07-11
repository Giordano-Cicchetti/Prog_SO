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
  List_init(&list);
  Add_message_to_list(&list,3,"ITALIA","feffo","gio");
  Add_message_to_list(&list,3,"INGHILTERRA","gio","feffo");
  Add_message_to_list(&list,3,"BELGIO","jack","feffo");
  printf(BWHT "size %d \n" reset,list.size);
  Remove_all_messages_from_list(&list);
  printf(BWHT "size %d \n" reset,list.size);

  Chat chat;
  ListHead list_msg;
  char user1[MAX_CREDENTIAL]="federico";
  char user2[MAX_CREDENTIAL]="giordano";
  Chat_create(&chat,user1,user2,&list_msg);
  Add_message_to_list(chat.list_msg,5,"BERRETTINI","feffo","gio");
  Add_message_to_list(chat.list_msg,5,"DJOKOVIC","gio","feffo");
  Add_message_to_list(chat.list_msg,5,"DJOKOVIC","feffo","gio");
  Add_message_to_list(chat.list_msg,5,"DJOKOVIC","gio","feffo");
  Chat_print(&chat);
  Chat_destroy(&chat);
}

