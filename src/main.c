#include "fs.h"
#include <stdio.h>

extern superblock* super;

int main() {
  format("p1", 1, 1024);
  read_inode(super->root_block)->file_size = 190;

  int i,pos;
  for (i=0; i<7; i++) {
    inode n = init_inode("test"+i, 1, 10);
    pos = alloc_block();
    write_inode(n, pos);
    printf("adding %d to dir.\n",pos);
    add_to_directory(super->root_block, pos);
  }
  print_superblock(*super);
//  int* block = (int*)get_position_pointer(read_inode(super->root_block)->indirect_ref);
//  print_block(block);
  printf("removing from dir:\n");
  remove_from_directory(super->root_block,2);
  print_superblock(*super);
//  block = (int*)get_position_pointer(read_inode(super->root_block)->indirect_ref);
//  print_block(block);

  return 0;
}
