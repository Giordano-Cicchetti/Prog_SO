#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "binary_file_search.h"
#include "structures.h"
#include "common.h"


//GC printing User
void User_print(int fd,int num_users){
  User u ;
  int i=num_users-1; // read in reverse order!!
  while(!binaryFileRead(fd, &u, sizeof(User), i)){
    printf("Username: %s, Password:%s\n", u.username, u.password);
    --i;
  }
}

//GC put all_usernames in buf 
void User_all_usernames(int fd,char* buf,int num_users) {
  User u ;
  buf[0]='\0';
  int i=num_users-1; // read in reverse order!!
  while(!binaryFileRead(fd, &u, sizeof(User), i)){
    strcat(buf,u.username);
    strcat(buf,"\n");
    --i;
  }
}

//GC searching a user in the file
int normalFileSearch(int fd, void* item, int item_size, CompareFn compare){
  //GC first we get the size of the file
  struct stat stats;
  fstat(fd, &stats);

  //GC from the size, we determine the number of records
  int size=stats.st_size;
  if(DEBUG) printf("Size: %d, Item_size: %d\n", size, item_size);
  

  //GC we determine the number of records and read from the right position 
  int num_records=size/item_size;
  assert(!(size%item_size));

  char buffer[item_size];
  int pos=0;

  while(num_records){
    off_t offset=lseek(fd, pos*item_size, SEEK_SET);
    if (offset<0){
      printf("Bad things happened\n");
      return -1;
    }
    int bytes_read=read(fd, buffer, item_size);
    if (bytes_read!=item_size){
      printf("Invalid read\n");
      return -1;
    }
    int cmp_result=(*compare)(item,buffer);
    if(!cmp_result) {
      return pos;
    }
    
    pos++;
    num_records--;
  }
  return -1;
}

//GC writing a user in the file
int binaryFileWrite(int fd, void* src, int item_size, int pos){
  off_t offset=lseek(fd, pos*item_size, SEEK_SET);
  if (offset<0) {
    ftruncate(fd, (pos+1)*item_size);
    offset=lseek(fd, pos*item_size, SEEK_SET);
  }
  int written_bytes=write(fd, src, item_size);
  if (item_size!=written_bytes)
    return -1;
  return 0;
}
 
//GC reading a user in the file
int binaryFileRead(int fd, void* dest, int item_size, int pos){
  size_t desired_offset=pos*item_size;
  off_t offset=lseek(fd, desired_offset, SEEK_SET);
  if (offset<0) {
    return -1;
  }
  int read_bytes=read(fd, dest, item_size);
  if (read_bytes!=item_size){
    return -1;
  }
  return 0;
}
