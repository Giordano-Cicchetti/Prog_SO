#include "structures.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "ANSI-color-codes.h"
#include "structures.h"
#include "binary_file_search.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//*USER*

//GC initializing the User
void User_init(User* dest,
		       char* username,
		       char* password){
  strcpy(dest->username, username);
  strcpy(dest->password, password);
}

//GC comparing Users
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

//GC comparing Users only by username
int User_compare_only_username(void* a, void* b){
  User* ua=(User*) a;
  User* ub=(User*) b;
  int cmp1=strcmp(ua->username, ub->username);
  
  return cmp1;
  
}


//*MESSAGE*

//GC initializing the Message 
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

//LISTS*

// FC initializing the list
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
  if (item->next || item->prev)
    return 0;
  
  // we check that the element is not in the list
  ListItem* instance=List_find(head, item);
  assert(!instance);

  // we check that the previous is inthe list

  if (prev) {
    ListItem* prev_instance=List_find(head, prev);
    assert(prev_instance);
  }
  // we check that the previous is inthe list


  ListItem* next= prev ? prev->next : head->first;
  if (prev) {
    item->prev=prev;
    prev->next=item;
  }
  if (next) {
    item->next=next;
    next->prev=item;
  }
  if (!prev)
    head->first=item;
  if(!next)
    head->last=item;
  ++head->size;
  return item;
}

//FC detach a list item from the list
ListItem* List_detach(ListHead* head, ListItem* item) {

  // we check that the element is in the list
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
void Add_message_to_list(ListHead head, int header, char* content, char* to, char* from){
  
  List_init(&head);
  
  MessageListItem* new_element= (MessageListItem*)malloc(sizeof(MessageListItem));
   
    if (! new_element) {
      printf("out of memory\n");
      exit(EXIT_FAILURE);
    }

    new_element->list.prev=0;
    new_element->list.next=0;
    new_element->msg=(Message*)malloc(sizeof(Message));
    new_element->msg->header=header;
    strcpy(new_element->msg->content, content);
    strcpy(new_element->msg->to, to);
    strcpy(new_element->msg->from, from);

    ListItem* result=
      List_insert(&head, head.last, (ListItem*) new_element);
    assert(result);
  
  if(DEBUG)
    MessageList_print(&head);
}

// FC remove all messages from the list in a chat
void Remove_all_messages_from_list(ListHead list){

    if(DEBUG)
        printf("removing all elements \n");
    ListItem* aux=list.first;
    int k=0;
    while(aux){
        ListItem* item=aux;
        aux=aux->next;
        List_detach(&list, item);
        free(((MessageListItem*)item)->msg);
        free(item);

        ++k;
    }

    if(DEBUG)
     MessageList_print(&list);

    }
