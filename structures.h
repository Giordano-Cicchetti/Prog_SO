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

//FC primitive functions to manage lists : IMPORTANT => before using a list remember to initialize it
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

//GC user struct size
#define USER_SIZE (int)sizeof(User)

//GC function to initialize the USER STRUCT
void User_init(User* dest,char* username,char* password);

//GC functions to compare the users. the first compares username and password, while the second compare only the username
//GC taking two items and returns -1 if v1<v2 ; 1 if v1>v2 ; 0 if v1==v2
int User_compare(void* a, void* b);
int User_compare_only_username(void* a, void* b);

// GC given a list of users, ask the user a username and check if it's avalaible
void checkregistereduser(char* listusers,char* interlocutor,char* current_user);

//######################################################################################################################

//*MESSAGE*

//FC message of n°header-type from (username) to (username) with a certain content
typedef struct {
    int header;
    char from[MAX_CREDENTIAL];
    char to[MAX_CREDENTIAL];
    char content[MAX_CONTENT];
} Message;

//FC message struct size
#define MESSAGE_SIZE (int)sizeof(Message)

//FC primitive functions to manage messages
void Message_init(Message* m,int header,char* from,char* to,void* content,int content_size);
void MessageList_print(ListHead* head);
void Remove_all_messages_from_list(ListHead* list);
void Add_message_to_list(ListHead* head, int header, char* content, char* from, char* to);

//######################################################################################################################

//*CHAT*

typedef struct {
    char user1[MAX_CREDENTIAL];
    char user2[MAX_CREDENTIAL];
    ListHead* list_msg;
} Chat;

//FC chat struct size
#define CHAT_SIZE (int)sizeof(Chat)

//FC primitive functions to manage chats
void Chat_create(Chat* chat,char user1[MAX_CREDENTIAL], char user2[MAX_CREDENTIAL], ListHead* list);
void Chat_destroy(Chat* chat);
void Chat_print(Chat* chat);
void Add_chat_to_list(ListHead* head, char user1[MAX_CREDENTIAL], char user2[MAX_CREDENTIAL], ListHead* msg_list);
void Remove_all_chats_from_list(ListHead* list);
void Chat_list_print(ListHead* list);
Chat* Find_chat_by_username(ListHead* chatlist, char username[MAX_CREDENTIAL]);
Chat* Chat_ispresent_between_users(ListHead* chatlist, char user1[MAX_CREDENTIAL], char user2[MAX_CREDENTIAL]);

//######################################################################################################################

//*USERONLINE*

//FC it represents the user while he/she is online, the chat in which he/she is and his/her IP address
typedef struct {
    char username[MAX_CREDENTIAL]; //FC taken from the message sent by the client to the server to enter a Chat
    char ipaddr[15]; //FC a string "10.0.0.1" for the IPv4 address taken from the sockaddr_in struct got when the server receives a message from a client who enters a Chat
    Chat* chat;
} UserOnline;

//FC useronline struct size
#define USERONLINE_SIZE (int)sizeof(UserOnline)

//FC primitive functions to manage users online
void UserOnline_create(UserOnline* useronline, char username[MAX_CREDENTIAL], char ipaddr[15], Chat* chat);
void UserOnline_print(UserOnline* useronline);
void UserOnline_list_print(ListHead* list);
void Add_useronline_to_list(ListHead* head, Chat* chat, char username[MAX_CREDENTIAL], char ipaddr[15]);
void Remove_useronline_from_list(ListHead* list, char username[MAX_CREDENTIAL]);
void Remove_all_useronline_from_list(ListHead* list);
char* Give_useronline_IP(ListHead* useronlinelist, char username[MAX_CREDENTIAL]);

//######################################################################################################################

//*LIST ITEMS: MESSAGE, CHAT and USERONLINE*
//FC We will use these items in ListHead to create different lists but with the same structure through polymorphism and inheritance

typedef struct MessageListItem{
  ListItem list;
  Message* msg; //FC pointer to message
} MessageListItem;

typedef struct ChatListItem{
  ListItem list;
  Chat* chat; //FC pointer to chat
} ChatListItem;

typedef struct UserOnlineListItem{
  ListItem list;
  UserOnline* useronline; //FC pointer to useronline
} UserOnlineListItem;

//######################################################################################################################

