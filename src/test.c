#include "test.h"
#include "dir_utils.h"
#include "file.h"
#include <stdio.h>

extern superblock* super;
extern void* partition;
void test_files(){
  int i,pos;
  // make a directory
  /*inode dir = init_inode("dev",4,0);
  pos = alloc_block();
  write_inode(dir,pos);
  add_to_directory(read_inode(super->root_block),pos);
  printf("dev directory position = %d\n", pos);
  dir = init_inode("sub_dir",4,0);
  int pos2 = alloc_block();
  write_inode(dir, pos2);
  add_to_directory(read_inode(pos),pos2);
  printf("sub_dir directory position = %d\n", pos2);
  for (i=0; i<7; i++) {
    inode n = init_inode("test"+i, 1, 10);
    pos = alloc_block();
    write_inode(n, pos);
    printf("adding %d to dir.\n",pos);
    add_to_directory(read_inode(super->root_block), pos);
  }*/
  if(mkdir("/dev",D_RW) == SUCCESS){
    printf("Making Directory /dev ==> SUCCESS\n\n");
  }
  char* dir = "/dev/sub_dir";
  char* files[2] = {"/dev/sub_dir/new_file","/dev/sub_dir/new_file2"};
  char* local_files[2] = {"files/local_file_1.txt","files/local_file_2.txt"};
  if(mkdir(dir,D_RW) == SUCCESS){
    printf("Making Directory %s ==> SUCCESS\n\n",dir);
  }
  printf("Number of free blocks %d\n\n",super->num_free_blocks);

  printf("Copying first file to dir %s.\n\n",dir);  
  copy_file(files[0],F_RW,local_files[0]);
  print_file(files[0]);
  
  printf("Number of free blocks %d\n\n",super->num_free_blocks);

  printf("Copying second file to dir %s.\n\n",dir);
  copy_file(files[1],F_RW,local_files[1]);
  print_file(files[1]);
  
  printf("Number of free blocks %d\n\n",super->num_free_blocks);

  printf("\nRemoving %s\n\n",files[1]);
  remove_file(files[1]);
  printf("Printing removed file.\n\n");
  int rm = print_file(files[1]);
  if( rm == 0){
    printf("File does not exist.\n\n");
  }

  printf("Number of free blocks %d\n\n",super->num_free_blocks);


  printf("\nRemoving %s\n\n",files[0]);
  remove_file(files[0]);
  printf("Printing removed file.\n\n");
  rm = print_file(files[0]);
  if( rm == 0){
    printf("File does not exist.\n\n");
  }

  printf("Number of free blocks %d\n\n",super->num_free_blocks);
}