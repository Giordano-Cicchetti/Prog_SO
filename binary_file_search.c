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



void User_print(int fd,int num_users){
  User u ;
  int i=num_users-1; // read in reverse order!!
  while(!binaryFileRead(fd, &u, sizeof(User), i)){
    printf("username: %s, password:%s\n", u.username, u.password);
    --i;
  }
}

int normalFileSearch(int fd, void* item, int item_size, CompareFn compare){
  //1 we get the size of the file
  struct stat stats;
  fstat(fd, &stats);

  // from the size, we determine the number of records
  int size=stats.st_size;
  if(DEBUG) printf("size: %d, item_size: %d\n", size, item_size);
  //printf("the file has size %d\n", (int)size);

  //we determine the number of records
  int num_records=size/item_size;
  assert(!(size%item_size));

  char buffer[item_size];
  int pos=0;

  while(num_records){
    off_t offset=lseek(fd, pos*item_size, SEEK_SET);
    if (offset<0){
      printf("bad things happened\n");
      return -1;
    }
    int bytes_read=read(fd, buffer, item_size);
    if (bytes_read!=item_size){
      printf("invalid read\n");
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