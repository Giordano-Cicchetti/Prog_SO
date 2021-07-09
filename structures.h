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

void MessageList_print(ListHead* head);
void Remove_all_messages_from_list(ListHead list);
void Add_message_to_list(ListHead head, int header, char* content, char* to, char* from);

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

//######################################################################################################################

//*CHAT*

typedef struct {
    User* user1;
    User* user2;
    ListHead list_msg;
} Chat;

#define CHAT_SIZE (int)sizeof(Chat)

//######################################################################################################################

//*USERONLINE*

//FC It represents the user while he/she is online, the chat in which he/she is and his/her IP address
typedef struct {
    User* user;
    char* ipaddr; //a string "10.0.0.1" for the IPv4 address
    Chat* chat;
} UserOnline;

#define USERONLINE_SIZE (int)sizeof(UserOnline)


//######################################################################################################################

//*LISTS: MESSAGE, CHAT and USERONLINE*

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

