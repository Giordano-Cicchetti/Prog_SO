#pragma once


//FC file to keep users registered
#define FILENAME "./registered_users"

//FC constants for username,password and message length
#define MAX_CREDENTIAL 128
#define MAX_CONTENT 1024


//######################################################################################################################

//*LISTS*

typedef struct ListItem {
  struct ListItem* prev;
  struct ListItem* next;
} ListItem;

typedef struct ListHead {
  ListItem* first;
  ListItem* last;
  int size;
} ListHead;

//FC primitive functions to manage lists
void List_init(ListHead* head);
ListItem* List_find(ListHead* head, ListItem* item);
ListItem* List_insert(ListHead* head, ListItem* previous, ListItem* item);
ListItem* List_detach(ListHead* head, ListItem* item);


//######################################################################################################################

//*USER*

//GC struct for the users to read them from the file, a struct UserOnline is used instead to keep users that are online. 
typedef struct {
  char username[MAX_CREDENTIAL];
  char password[MAX_CREDENTIAL];
} User;

#define USER_SIZE (int)sizeof(User)

//GC function to initialize the USER STRUCT
void User_init(User* dest,char* username,char* password);
//GC functions to compare the users. the first compares username and password, while the second compare only the username
//GC It two items and returns -1 if v1<v2 ; 1 if v1>v2 ; 0 if v1==v2
int User_compare(void* a, void* b);
int User_compare_only_username(void* a, void* b);


//######################################################################################################################

//*MESSAGE*

typedef struct {
    int header;
    char from[MAX_CREDENTIAL];
    char to[MAX_CREDENTIAL];
    char content[MAX_CONTENT];
} Message;

#define MESSAGE_SIZE (int)sizeof(Message)

void Message_init(Message* m,int header,char* from,char* to,void* content,int content_size);
void MessageList_print(ListHead* head);
void Remove_all_messages_from_list(ListHead* list);
void Add_message_to_list(ListHead* head, int header, char* content, char* from, char* to);
void Remove_all_chats_from_list(ListHead* list);

//######################################################################################################################

//*CHAT*

typedef struct {
    char user1[MAX_CREDENTIAL];
    char user2[MAX_CREDENTIAL];
    ListHead* list_msg;
} Chat;

#define CHAT_SIZE (int)sizeof(Chat)

void Chat_create(Chat* chat,char user1[MAX_CREDENTIAL], char user2[MAX_CREDENTIAL], ListHead* list);
void Chat_destroy(Chat* chat);
void Chat_print(Chat* chat);
void Add_chat_to_list(ListHead* head, char user1[MAX_CREDENTIAL], char user2[MAX_CREDENTIAL], ListHead* msg_list);
void Chat_list_print(ListHead* list);

//######################################################################################################################

//*USERONLINE*

//FC It represents the user while he/she is online, the chat in which he/she is and his/her IP address
typedef struct {
    char username[MAX_CREDENTIAL]; //FC taken from the message sent by the client to the server to enter a Chat
    char ipaddr[15]; //FC a string "10.0.0.1" for the IPv4 address taken from the sockaddr_in struct got when the server receives a message from a client who enters a Chat
    Chat* chat;
} UserOnline;

#define USERONLINE_SIZE (int)sizeof(UserOnline)


//######################################################################################################################

//*LIST ITEMS: MESSAGE, CHAT and USERONLINE*
// FC We will use these items in ListHead to create different lists but with the same structure through polymorphism

typedef struct MessageListItem{
  ListItem list;
  Message* msg;
} MessageListItem;

typedef struct ChatListItem{
  ListItem list;
  Chat* chat;
} ChatListItem;

typedef struct UserOnlineListItem{
  ListItem list;
  UserOnline* useronline;
} UserOnlineListItem;

//######################################################################################################################

//*RECEIVER_THREAD_ARG*

typedef struct handler_args_s
{
    int socket_desc;
} handler_args_t;
