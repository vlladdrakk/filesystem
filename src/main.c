#include "fs.h"
#include "common.h"
#include <stdio.h>

extern superblock* super;
extern void* partition;

int main() {
  format("p1", 1, 64);
  test_dirs();
  test_disk_utils(partition);

  return 0;
}
