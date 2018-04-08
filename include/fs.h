#ifndef FS_H
#define FS_H

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

void print_partition();

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

int get_inode_pos(inode* node);

int is_writable(inode* node);

int is_readonly(inode* node);

int is_dir(inode* node);

int is_file(inode* node);

void* format(char* name, char flags, int num_blocks);

extern int dump_to_disk(void* partition, char* text_file);

extern void* load_from_disk(char* text_file);

extern void test_disk_utils(void* partition);

extern int mkdir(char* name, char flags);

extern int rmdir(char* name);

extern void test_dirs();

extern int ls(char* path);

#endif