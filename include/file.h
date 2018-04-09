#ifndef FILE_H
#define FILE_H
#include "common.h"
#include "fs.h"

int find_node(char** name, int size);

int copy_file(char* name, char flags, char* local_file);

int remove_file(char* name);

int print_file(char* name);

#endif