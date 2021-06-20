#include "structures.h"
#include <string.h>

void User_init(User* dest,
		       char* username,
		       char* password){
  strcpy(dest->username, username);
  strcpy(dest->password, password);
}

void Message_init(Message* m,
                int header,
		            char* from,
		            char* to,
                void* content,
                int content_size){

    m->header=header;

    if(from==NULL) m->from[0]='\0';
    else strcpy(m->from, from);

    if(to==NULL) m->to[0]='\0';
    else strcpy(m->to, to);
    
    
    memcpy(m->content,content,content_size);

}


int User_compare(void* a, void* b){
  User* ua=(User*) a;
  User* ub=(User*) b;
  int cmp1=strcmp(ua->username, ub->username);
  if(cmp1==0){
    int cmp2 =strcmp(ua->password,ub->password);
    return cmp2;
  }
  return cmp1;
  
}
int User_compare_only_username(void* a, void* b){
  User* ua=(User*) a;
  User* ub=(User*) b;
  int cmp1=strcmp(ua->username, ub->username);
  
  return cmp1;
  
}