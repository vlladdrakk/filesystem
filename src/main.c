#include "fs.h"
#include "common.h"
#include <stdio.h>

extern superblock* super;

int main() {
  format("p1", 1, 64);
  test_dirs();

  return 0;
}
