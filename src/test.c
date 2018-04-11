#include "test.h"
#include "dir_utils.h"
#include "file.h"
#include "common.h"
#include <stdio.h>

extern superblock* super;
extern void* partition;
void test_files(){
  puts("TEST FILES\n");
  partition = format("new partition",1,2048);
  
  if(mkdir("/var",D_RW) == SUCCESS){
    printf("Making Directory /dev ==> SUCCESS\n\n");
  }
  char* dir = "/var/log";
  char* files[2] = {"/var/log/new_file","/var/new_file2"};
  char* local_files[2] = {"files/local_file_1.txt","files/local_file_2.txt"};
  if(mkdir("/var/log",D_RW) == SUCCESS ){
    printf("Making Directory /var/log ==> SUCCESS\n\n");
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