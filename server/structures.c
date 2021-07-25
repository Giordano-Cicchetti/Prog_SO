#include "common.h"
#include "ANSI-color-codes.h"
#include "structures.h"
#include "binary_file_search.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>  // htons()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>


//*USER*

//GC initializing the user
void User_init(User* dest,
		       char* username,
		       char* password){
  memset(dest,0,USER_SIZE);
  strcpy(dest->username, username);
  strcpy(dest->password, password);
}

//GC comparing users
int User_compare(void* a, void* b){
  User* ua=(User*) a;
  User* ub=(User*) b;
  int cmp1=strcmp(ua->username, ub->username);
  if(cmp1==0){
    int cmp2=strcmp(ua->password,ub->password);
    return cmp2;
  }
  return cmp1;
  
}

//GC comparing users only by username
int User_compare_only_username(void* a, void* b){
  User* ua=(User*) a;
  User* ub=(User*) b;
  int cmp1=strcmp(ua->username, ub->username);
  return cmp1;
}

//GC given a list of users, ask the user a username and check if it's avalaible
void Check_registered_user(char* listusers,char* interlocutor,char* current_user){
  char user_in[MAX_CREDENTIAL];
  int user_len=0;
  char* quit=SERVER_COMMAND;
  int quit_len=strlen(quit);
  const char s = '\n';
  char list[strlen(listusers)+1];
  
  while(1){
    //GC take the username
    strcpy(list,listusers); 
    printf(BGRN "You have to insert the username of the user with whom you want to talk\n");
    printf(BBLU "\nChoose between these users registered to our Private Chat:\n\n%s",listusers);
    printf(BBLU "\nInsert it here: ");
    scanf("%s",user_in);
    while(getchar()!='\n');
    user_len = strlen(user_in);
    if (user_len == quit_len -1  && !memcmp(user_in, quit, quit_len -1)){
      if (DEBUG) fprintf(stderr,BWHT "Sent QUIT command ...\n");
      strcpy(interlocutor, quit);
      return;
    }
    if(!strcmp(current_user,user_in)){
      printf(BRED "\nYou can't talk with yourself!\n\n");
      continue;
    }
    char *token;
    //GC get the first token 
    token = strtok(list, &s);
    
    //GC walk through other tokens 
    while( token != NULL ) {
      if(strcmp(user_in,token)==0){
      strcpy(interlocutor, user_in);
      return;
      }
      token = strtok(NULL, &s);
    }
    printf(BWHT "Username not found!\n");
  }
}

//######################################################################################################################

//*MESSAGE*

//GC initializing the message 
void Message_init(Message* m,
                int header,
		            char* from,
		            char* to,
                void* content,
                int content_size){
    memset(m,0,MESSAGE_SIZE);
    m->header=header;

    if(from==NULL) m->from[0]='\0';
    else strcpy(m->from, from);

    if(to==NULL) m->to[0]='\0';
    else strcpy(m->to, to);
    
    
    memcpy(m->content,content,content_size);

}


// FC printing the messages in a messages list
void MessageList_print(ListHead* head){
  ListItem* aux=head->first;
  printf(BRED "[ \n");
  while(aux){
    MessageListItem* element = (MessageListItem*) aux;
    printf(BWHT "Message:%s \n" reset, element->msg->content);
    if (DEBUG){
      printf(BBLK  "Header:%d \n" reset, element->msg->header);
      printf(BBLK  "From:%s \n" reset, element->msg->from);
      printf(BBLK  "To:%s \n" reset, element->msg->to);
    }
    aux=aux->next; 
  }
  printf(BRED "]\n");
}


// FC add a message to the list of messages in a chat
void Add_message_to_list(ListHead* head, int header, char* content, char* from, char* to){
  MessageListItem* new_element= (MessageListItem*)malloc(sizeof(MessageListItem));
  if (!new_element) {
      printf(BWHT "Out of memory\n");
      exit(EXIT_FAILURE);
    }

  (new_element->list).next=NULL;
  (new_element->list).prev=NULL;
  new_element->msg=(Message*)malloc(sizeof(Message));
   
  Message_init(new_element->msg,header,from,to,content,MAX_CONTENT);
   

  ListItem* result= List_insert(head,head->last, (ListItem*) new_element);
  assert(result);
  
  if(DEBUG)
    MessageList_print(head);
}

// FC remove all messages from the list in a chat (memory safe)
void Remove_all_messages_from_list(ListHead* list){
  
  printf(BWHT "....Removing all messages from the list.... \n \n");

  ListItem* aux=list->first;
  while(aux){
        ListItem* item=aux;
        aux=aux->next;
        List_detach(list, item);
        free(((MessageListItem*)item)->msg);
        free(item);
  }

  if(DEBUG)
     MessageList_print(list);
  }
//GC FC send to the client all the messages in the chat
void MessageList_send(ListHead* head,int sd,void* client_addr){
  ListItem* aux=head->first;
  int bytes_sent=0 , ret ;
  while(aux){
    MessageListItem* element = (MessageListItem*) aux;
    Message* m= element->msg;
    
    bytes_sent=0;
    while ( bytes_sent < MESSAGE_SIZE) {
      ret = sendto(sd, m, MESSAGE_SIZE, 0, (struct sockaddr*) client_addr, sizeof(struct sockaddr_in));
      if (ret == -1 && errno == EINTR) continue;
      if (ret == -1) handle_error("Cannot write to the socket");
      bytes_sent = ret;
    }

    aux=aux->next; 
  }
  
}

//######################################################################################################################

//LISTS*

//FC initializing the list
void List_init(ListHead* head) {
  head->first=0;
  head->last=0;
  head->size=0;
}

//FC finding a list item in the list
ListItem* List_find(ListHead* head, ListItem* item) {
  // linear scanning of list
  ListItem* aux=head->first;
  while(aux){
    if (aux==item)
      return item;
    aux=aux->next;
  }
  return 0;
}

//FC inserting a list item to the list

ListItem* List_insert(ListHead* head, ListItem* prev, ListItem* item) {
  if (item->next || item->prev){
    return 0;
  }
  
  //FC we check that the element is not in the list
  ListItem* instance=List_find(head, item);
  assert(!instance);

  //FC we check that the previous is inthe list
  if (prev) {
    ListItem* prev_instance=List_find(head, prev);
    assert(prev_instance);
  }
  //FC we check that the previous is in the list


  ListItem* next= prev ? prev->next : head->first;
  if (prev) {
    item->prev=prev;
    prev->next=item;
     
  }
  if (next) {
    item->next=next;
    next->prev=item;
     
  }
  if (!prev){
    head->first=item;
    
    }
  if(!next){
    head->last=item;
  }
  (head->size)+=1;
  return item;
}

//FC detach a list item from the list
ListItem* List_detach(ListHead* head, ListItem* item) {

  //FC we check that the element is in the list
  ListItem* instance=List_find(head, item);
  assert(instance);


  ListItem* prev=item->prev;
  ListItem* next=item->next;
  if (prev){
    prev->next=next;
  }
  if(next){
    next->prev=prev;
  }
  if (item==head->first)
    head->first=next;
  if (item==head->last)
    head->last=prev;
  head->size--;
  item->next=item->prev=0;
  return item;
}

//######################################################################################################################

//*CHAT*

//FC chat creator giving two users,a Chat struct
void Chat_create(Chat* chat,char user1[MAX_CREDENTIAL], char user2[MAX_CREDENTIAL], ListHead* list){
  assert(chat);
  strcpy(chat->user1,user1);
  strcpy(chat->user2,user2);
  List_init(list);
  chat->list_msg=list;
}

//FC chat destroyer 
void Chat_destroy(Chat* chat){
  printf(BGRN "Destroying chat between %s and %s \n" reset,chat->user1,chat->user2);
  Remove_all_messages_from_list(chat->list_msg);
  free(chat->list_msg);
  free(chat);
}

//FC printer of chat messages
void Chat_print(Chat* chat){
  printf(BGRN "Chat between %s and %s \n" reset,chat->user1,chat->user2);
  if(chat->list_msg != NULL)
    MessageList_print(chat->list_msg);   
}

//FC adding a chat to the list kept by the server giving users of the Chat and an empty list of messages
void Add_chat_to_list(ListHead* head, char user1[MAX_CREDENTIAL], char user2[MAX_CREDENTIAL], ListHead* msg_list){
  ChatListItem* new_element= (ChatListItem*)malloc(sizeof(ChatListItem));
  if (!new_element) {
      printf(BWHT "Out of memory \n");
      exit(EXIT_FAILURE);
  }

  (new_element->list).next=NULL;
  (new_element->list).prev=NULL;
  new_element->chat=(Chat*)malloc(sizeof(Chat));
  Chat_create(new_element->chat,user1,user2,msg_list);
   
  ListItem* result= List_insert(head,head->last, (ListItem*) new_element);
  assert(result);
  
  if(DEBUG)
    Chat_print(new_element->chat);
}

//FC printer of the chats list
void Chat_list_print(ListHead* list){
  ListItem* aux=list->first;
  while(aux){
    Chat_print(((ChatListItem*)aux)->chat);
    aux=aux->next;
      }
}

//FC to remove all chats from a list of chats
void Remove_all_chats_from_list(ListHead* list){
  

  printf(BWHT "....Removing all chats from the list.... \n \n");
  
  ListItem* aux=list->first;
  while(aux){
    ListItem* item=aux;
    aux=aux->next;
    List_detach(list, item);
    Chat_destroy(((ChatListItem*)item)->chat);
    free(item);
  }
  
  if(DEBUG)
     Chat_list_print(list);

}

//FC finding a chat by username involved in it
Chat* Find_chat_by_username(ListHead* chatlist, char username[MAX_CREDENTIAL]){
  if(DEBUG)
    printf(BWHT "....Finding a chat where %s is involved.... \n \n", username);
  
  ListItem* aux=chatlist->first;
  while(aux){
    ChatListItem* item=(ChatListItem*)aux;
    Chat* chat=item->chat;
    if (strcmp(chat->user1, username) == 0 || strcmp(chat->user2, username) == 0)
        return chat;

    aux=aux->next;
}
  return NULL;
}

//FC finding a chat by usernames involved in it
Chat* Chat_ispresent_between_users(ListHead* chatlist, char user1[MAX_CREDENTIAL],char user2[MAX_CREDENTIAL]){
  if(DEBUG)
    printf(BWHT "....Finding a chat where %s and %s are involved.... \n \n", user1, user2);
  
  ListItem* aux=chatlist->first;
  while(aux){
    ChatListItem* item=(ChatListItem*)aux;
    Chat* chat=item->chat;
    if ((strcmp(chat->user1, user1) == 0 && strcmp(chat->user2, user2) == 0) || (strcmp(chat->user1, user2) == 0 && strcmp(chat->user2, user1) == 0))
        return chat;

    aux=aux->next;
}
  return NULL;
}

//######################################################################################################################

//*USERONLINE*

//FC initializing a useronline
void UserOnline_create(UserOnline* useronline, char username[MAX_CREDENTIAL], char ipaddr[15],in_port_t port, Chat* chat){
  assert(useronline);
  strcpy(useronline->username,username);
  strcpy(useronline->ipaddr,ipaddr);
  useronline->chat=chat;
  useronline->port=port;
}

//FC printing a user online with infos
void UserOnline_print(UserOnline* useronline){
  char otheruser[MAX_CREDENTIAL];
  if (!strcmp((useronline->chat)->user1, useronline->username)){
    strcpy(otheruser, useronline->chat->user2);
  }
  else{
    strcpy(otheruser, useronline->chat->user1);
  }
  printf(BGRN "\nUser %s is online through this IP address: %s \nHe/She is talking with %s\n" reset, useronline->username, useronline->ipaddr, otheruser);
  if(useronline->chat->list_msg != NULL){
    printf(BRED "\nList of messages:\n");
    MessageList_print(useronline->chat->list_msg);  
  }
}

//FC printing all users online in the list of the server
void UserOnline_list_print(ListHead* list){
  ListItem* aux=list->first;
  while(aux){
    UserOnline_print(((UserOnlineListItem*)aux)->useronline);
    aux=aux->next;
  }
}

//FC adding a user online to the global list in the server memory when enters a chat
void Add_useronline_to_list(ListHead* head, Chat* chat, char username[MAX_CREDENTIAL], char ipaddr[15],in_port_t port){
  UserOnlineListItem* new_element= (UserOnlineListItem*)malloc(sizeof(UserOnlineListItem));
  if (!new_element) {
      printf(BWHT "Out of memory \n");
      exit(EXIT_FAILURE);
  }

  (new_element->list).next=NULL;
  (new_element->list).prev=NULL;
  new_element->useronline=(UserOnline*)malloc(sizeof(UserOnline));
  UserOnline_create(new_element->useronline,username,ipaddr,port,chat);
   
  ListItem* result= List_insert(head,head->last, (ListItem*) new_element);
  assert(result);
  
  if(DEBUG)
    UserOnline_print(new_element->useronline);
}

//FC removing a specific user online from a list
void Remove_useronline_from_list(ListHead* list, char username[MAX_CREDENTIAL]){
  if(DEBUG)
    printf(BWHT "....Now %s is offline.... \n \n", username);
  
  ListItem* aux=list->first;
  while(aux){
    ListItem* item=aux;
    if (strcmp((((UserOnlineListItem*)item)->useronline)->username, username) == 0){
      List_detach(list, item);
      free(((UserOnlineListItem*)item)->useronline);
      free((UserOnlineListItem*)item);
      return;
    }
    else{
    aux=aux->next;
    continue;
    }
  }
  if(DEBUG)
     UserOnline_list_print(list);
  }

//FC to remove all users online from a list of users online in the server
void Remove_all_usersonline_from_list(ListHead* list){
  
  printf(BWHT "....Removing all users online from the list.... \n \n");
  
  ListItem* aux=list->first;
  while(aux){
    ListItem* item=aux;
    aux=aux->next;
    List_detach(list, item);
    free(((UserOnlineListItem*)item)->useronline);
    free((UserOnlineListItem*)item);
  }
  if(DEBUG)
     UserOnline_list_print(list);

}

//FC giving back the IP address of a username
char* Give_useronline_IP(ListHead* useronlinelist, char username[MAX_CREDENTIAL]){
  
  ListItem* aux=useronlinelist->first;

  while(aux){
    UserOnlineListItem* item=(UserOnlineListItem*)aux;
    if (strcmp(item->useronline->username, username)== 0){
      if(DEBUG)
        printf(BWHT "....Giving IP of %s: %s  .... \n \n", username, item->useronline->ipaddr);
      return item->useronline->ipaddr;
    }

    aux=aux->next;
  }
  return NULL;
}

//FC giving back the username if it is online or NULL otherwise
char* UserOnline_ispresent(ListHead* useronlinelist, char username[MAX_CREDENTIAL]){

  ListItem* aux=useronlinelist->first;

  while(aux){
    UserOnlineListItem* item=(UserOnlineListItem*)aux;
    if (strcmp(item->useronline->username, username)== 0){
      if(DEBUG)
        printf(BRED "....%s is online!  .... \n\n", username);
      return item->useronline->username;
    }

    aux=aux->next;
  }
  return NULL;

}
in_port_t Give_useronline_Port(ListHead* useronlinelist, char username[MAX_CREDENTIAL]){

  ListItem* aux=useronlinelist->first;

  while(aux){
    UserOnlineListItem* item=(UserOnlineListItem*)aux;
    if (strcmp(item->useronline->username, username)== 0){
      if(DEBUG)
        printf(BWHT "....Giving IP of %s: %s  .... \n \n", username, item->useronline->ipaddr);
      return item->useronline->port;
    }

    aux=aux->next;
  }
  return 0;
}

Chat* Give_useronline_Chat(ListHead* useronlinelist, char username[MAX_CREDENTIAL]){

  ListItem* aux=useronlinelist->first;

  while(aux){
    UserOnlineListItem* item=(UserOnlineListItem*)aux;
    if (strcmp(item->useronline->username, username)== 0){
      if(DEBUG)
        printf(BWHT "....Giving IP of %s: %s  .... \n \n", username, item->useronline->ipaddr);
      return item->useronline->chat;
    }

    aux=aux->next;
  }
  return NULL;
}



