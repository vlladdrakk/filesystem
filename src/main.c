#include "fs.h"
#include "common.h"
#include <stdio.h>

extern superblock* super;

int main() {
  format("p1", 1, 1024);
  test_dirs();

  return 0;
}
