#pragma once

#define FILENAME "./Registred_User"
#define MAX_CREDENTIAL 128
#define MAX_CONTENT 1024

//GC struct for the user. 
typedef struct {
  char username[MAX_CREDENTIAL];
  char password[MAX_CREDENTIAL];
} User ;

#define USER_SIZE (int)sizeof(User)

//GC function to initialize the USER STRUCT
void User_init(User* dest,char* username,char* password);
//GC functions to compare the users. the first compare username and password
//while the sencond compare only the username
// compare function, takes two items and returns
// -1 if v1<v2
// 1 if v1>v2
// 0 if v1==v2
int User_compare(void* a, void* b);
int User_compare_only_username(void* a, void* b);





typedef struct {
    int header;
    char from[MAX_CREDENTIAL];
    char to[MAX_CREDENTIAL];
    char content[MAX_CONTENT];
} Message;

#define MESSAGE_SIZE (int)sizeof(Message)

void Message_init(Message* m,int header,char* from,char* to,void* content,int content_size);