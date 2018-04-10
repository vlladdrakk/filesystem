#ifndef COMMON_H
#define COMMON_H

#define SUCCESS 0
#define FAILURE 1
#define BLK_SIZE 1024
#define MAX_DREFS 190
#define MAX_DIRS MAX_DREFS + (BLK_SIZE/sizeof(int))
#define MIN_NUM_BLOCKS 32
#define MAX_NUM_BLOCKS 6048

// Permission definitions
#define F_RO 0 // File Read Only
#define F_RW 1 // File Read Write
#define D_RO 3 // Directory Read Only
#define D_RW 4 // Directory Read Write

char** strsplit(char* str, const char* delimiter);

int str_split (const char *str, char *parts[], const char *delimiter);

#endif