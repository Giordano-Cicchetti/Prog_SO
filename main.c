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


//FC to test the functions created during the development
int main(int argc, char** argv) {
  
  ListHead list; 
  List_init(&list);
  Add_message_to_list(&list,3,"ITALIA","feffo","gio");
  Add_message_to_list(&list,3,"INGHILTERRA","gio","feffo");
  Add_message_to_list(&list,3,"BELGIO","jack","feffo");
  printf(BWHT "size %d \n" reset,list.size);
  Remove_all_messages_from_list(&list);
  printf(BWHT "size %d \n \n" reset,list.size);
////////////////////


//FC how to create a chat: create a list of message,initialize it and then use the Add_chat_to_list function

  ListHead list_msg1; 
  ListHead list_msg2;
  ListHead list_chat;
  List_init(&list_msg1); //FC remember to initialize a list before using it
  List_init(&list_chat);
  List_init(&list_msg2);

  char user1[MAX_CREDENTIAL]="federico";
  char user2[MAX_CREDENTIAL]="giordano";

  char user3[MAX_CREDENTIAL]="jacopo";
  char user4[MAX_CREDENTIAL]="filippo";

  Add_chat_to_list(&list_chat, user1, user2, &list_msg1);  //FC it prints the Chat that is empty at the beginning
  Add_chat_to_list(&list_chat, user3, user4, &list_msg2); 


  Add_message_to_list(&list_msg1,5,"BERRETTINI","feffo","gio"); //FC then it fills the Chat
  Add_message_to_list(&list_msg1,5,"DJOKOVIC","gio","feffo");
  Add_message_to_list(&list_msg1,5,"DJOKOVIC","feffo","gio");
  Add_message_to_list(&list_msg1,5,"DJOKOVIC","gio","feffo");

  Add_message_to_list(&list_msg2,3,"ITALIA","feffo","gio");
  Add_message_to_list(&list_msg2,3,"INGHILTERRA","gio","feffo");
  Add_message_to_list(&list_msg2,3,"BELGIO","jack","feffo");


  Chat_list_print(&list_chat);
  
  //FC to print a single chat : Chat_print(((ChatListItem*)(list_chat.first))->chat);


  Remove_all_chats_from_list(&list_chat);

}

