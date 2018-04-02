#include "fs.h"
#include "common.h"
#include <stdio.h>

extern superblock* super;

int main() {
  format("p1", 1, 1024);

  // Create /etc
  // read_inode(super->root_block)->file_size = 190;
  int block = alloc_block();
  inode etc = init_inode("etc", 4, 0);
  write_inode(etc, block);

  add_to_directory(1, block);

  mkdir("/Users/thomas/home", 1);

  return 0;
}
