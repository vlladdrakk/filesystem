#include "fs.h"
#include <stdio.h>

extern superblock* super;

int main() {
  format("p1", 1, 1024);
  test_files();  
  return 0;
}
