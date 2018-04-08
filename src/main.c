#include "fs.h"
#include <stdio.h>

extern superblock* super;
void free_fields(char ** options)
{
    int j=0;
    while( options[j] != NULL){
      free(options[j]);
      j++;      
    }
    free(options);
}
int main() {
  format("p1", 1, 1024);
  //read_inode(super->root_block)->file_size = 190;
  int i,pos;
  // make a directory
  inode dir = init_inode("dev",4,0);
  pos = alloc_block();
  write_inode(dir,pos);
  add_to_directory(super->root_block,pos);
  printf("dev directory position = %d\n", pos);
  dir = init_inode("sub_dir",4,0);
  int pos2 = alloc_block();
  write_inode(dir, pos2);
  add_to_directory(pos,pos2);
  printf("sub_dir directory position = %d\n", pos2);


  for (i=0; i<7; i++) {
    inode n = init_inode("test"+i, 1, 10);
    pos = alloc_block();
    write_inode(n, pos);
    printf("adding %d to dir.\n",pos);
    add_to_directory(super->root_block, pos);
  }

  //print_superblock(*super);
  // char* name = "dev/sub_dir/new_file";
  // for ( i = 0; i < 5 ; i++){
  //   printf("i = %d\n",i);
  //   char** arr = strsplit(name,"/");
  //   // char* arr[10];
  //   // size_t size = str_split(name, arr, "/");
  //   // int i = 0;
  //   // for (; i < size; ++i) {
  //   //   printf("%s\n", arr[i]);
  //   // }
  //   while(arr[i] != NULL){
  //     i++
  //   }
  //   for (int j = 0; j < i; j++)
  //     free(arr[j]);
  //   printf("\n");
  //   printf("%s\n",name);

  // }
  printf("Copying first file.\n\n");  
  copy_file("/dev/new_file",1,"files/local_file_1.txt");
  print_file("/dev/new_file");
  
  printf("Copying second file.\n\n");
  copy_file("/dev/sub_dir/new_file2",1,"files/local_file_1.txt");
  print_file("/dev/sub_dir/new_file2");
  
  print_superblock(*super);
  printf("\nRemoving new_file2\n\n");
  remove_file("/dev/sub_dir/new_file2");
  print_superblock(*super);
  return 0;
}
