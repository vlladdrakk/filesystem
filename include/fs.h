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
	unsigned char block_map[756]; /* bit map of used(1)/free(0) blocks */
} superblock;

typedef struct {
	char filename[255];
	char flags; /* file: 0–read; 1–write; dir: 3–read; 4-write*/int file_size; /* number of bytes in the file */
	int direct_refs[190]; /* direct references */
	int indirect_ref; /* indirect reference */
} inode;

void print_block(int* block);

void print_superblock(superblock sblock);

void* get_position_pointer(int pos);

void read_super(void* ptr);

void write_super(superblock s_block);

inode init_inode(char* name, char flags, int file_size);

void write_inode(inode inode_in, int pos);

inode* read_inode(int pos);

void reserve_block(int pos);

void free_block(int pos);

int check_block(int pos);

int alloc_block();

void add_to_directory(int directory_pos, int inode_pos);

void remove_from_directory(int directory_pos,int inode_pos);

void* format(char* name, char flags, int num_blocks);

int mkdir(char* name, char flags);

#endif