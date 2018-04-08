#include "fs.h";

extern superblock* super;
extern void* partition;

void test_files(){
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

  print_superblock(*super);
  printf("Copying first file.\n\n");  
  copy_file("/dev/new_file",1,"files/local_file_1.txt");
  print_file("/dev/new_file");
  
  printf("Copying second file.\n\n");
  copy_file("/dev/sub_dir/new_file2",1,"files/local_file_2.txt");
  print_file("/dev/sub_dir/new_file2");
  
  print_superblock(*super);
  printf("\nRemoving new_file2\n\n");
  remove_file("/dev/sub_dir/new_file2");
  print_superblock(*super);
}