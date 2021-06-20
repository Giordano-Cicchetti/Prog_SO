
typedef int (*CompareFn)(void* v1, void* v2);

//GC function to print all the users in the file mapped in the file descriptor
void User_print(int fd,int num_users);


//GC returns the position of an an item od size item_size
//within a file of records
//to compare the items the function uses the compare function
//pointer
int normalFileSearch(int fd, void* item, int item_size, CompareFn compare);

// GC writes a record of size item_size in position pos in the file
// if the file is too small, it gets enlarged
// returns the number of bytes written
int binaryFileWrite(int fd, void* item, int item_size, int pos);  // tbd

// GC writes a record of size item_size in position pos in the file
// if the file is too small, it gets enlarged
// returns the number of bytes read or a negative number
// if error
int binaryFileRead(int fd, void* item, int item_size, int pos);   // tbd


