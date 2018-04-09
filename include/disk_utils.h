#ifndef DISK_UTILS_H
#define DISK_UTILS_H

#define byte unsigned char

int dump_to_disk(void* partition, char* text_file);

void* load_from_disk(char* text_file);

#endif