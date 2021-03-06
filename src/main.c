#include "fs.h"
#include "common.h"
#include "test.h"
#include <stdio.h>

extern superblock* super;
extern void* partition;

int main() {
  format("p1", 1, 1024);
  test_dirs();
  test_disk_utils(partition);
  test_files(); 
  return 0;
}
