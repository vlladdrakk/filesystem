#ifndef FS_H
#define FS_H

#define SUCCESS 0
#define FAILURE 1
#define BLK_SIZE 1024

typedef struct {
	char name[255];
	char flags; /* 0 – read; 1 – write */
	int num_blocks; /* number of blocks in the partition */
	int root_block; /* block number of the root inode */
	int num_free_blocks; /* number of blocks that are free */
	char block_map[756]; /* bit map of used(1)/free(0) blocks */
} superblock;

typedef struct {
	char filename[255];
	char flags; /* file: 0–read; 1–write; dir: 3–read; 4-write*/int file_size; /* number of bytes in the file */
	int direct_refs[190]; /* direct references */
	int indirect_ref; /* indirect reference */
} inode;

#endif