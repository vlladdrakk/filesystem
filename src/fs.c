#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "fs.h"

superblock* super;
void* partition;

void print_block(int* block) {
	int i;
	for (i = 0; i < BLK_SIZE; i++) {
		printf("%d", block[i]);
	}
}

void print_superblock(superblock sblock) {
	printf("name: %s\nflags: %d\nroot_block: %d\nnum_free_blocks: %d\nblock_map %d-%d\n",
		sblock.name, sblock.flags, sblock.root_block, sblock.num_free_blocks, sblock.block_map[1],sblock.block_map[0]);
}
void print_node( inode node){
	printf("name: %s\nflags: %d\nfile_size: %d\ndirect_refs: %n\nindirect_refs %d\n",
		node.filename, node.flags, node.file_size, node.direct_refs, node.indirect_ref);	
}

void print_partition() {
	int x = 30;
	int y = (super->num_blocks / x) + 1;
	int pos = 0;
	puts("Block Map:");

	int i, j;
	for (i = 0; i < y; i++) {
		for (j = 0; j < x; j++) {
			if (pos > super->num_blocks)
				printf("x ");
			else
				printf("%d ", check_block(pos));

			pos++;
		}
		printf("\n");
	}
}

void* get_position_pointer(int pos) {
	return partition + (pos * BLK_SIZE);
}

int get_inode_pos(inode* node) {
	return ((void*)node - partition) / BLK_SIZE;
}

void read_super(void* ptr) {
	super = (superblock*)ptr;
}

void write_super(superblock s_block) {
	super = (superblock*)get_position_pointer(0);
	*super = s_block;
}

inode init_inode(char* name, char flags, int file_size) {
	inode new_inode;
	strncpy(new_inode.filename, name, strlen(name));
	new_inode.filename[strlen(name)] = '\0'; // Add null terminator
	new_inode.flags = flags;
	new_inode.file_size = file_size;
	memset(new_inode.direct_refs, 0, sizeof(int)*MAX_DREFS);
	new_inode.indirect_ref = 0;

	return new_inode;
}

void write_inode(inode inode_in, int pos) {
	inode* node = (inode*)get_position_pointer(pos);
	*node = inode_in;
}

inode* read_inode(int pos) {
	return (inode*)get_position_pointer(pos);
}

void reserve_block(int pos) {
	if (super != NULL && check_block(pos) == 1) {
		printf("Error! Attempting to reserve a reserved block: %d\n", pos);
		exit(1);
	}

	super->block_map[pos/8] |= 1 << pos % 8;
	super->num_free_blocks--;
}

void free_block(int pos) {
	super->block_map[pos/8] &= ~(1 << pos % 8);
	super->num_free_blocks++;
}

int check_block(int pos) {
	return (super->block_map[pos/8] >> pos % 8) & 1;
}

int alloc_block() {
	int pos = -1;
	// Find free block
	int i;
	for (i = 0; i < super->num_blocks; i++) {
		if (check_block(i) == 0) {
			pos = i;
			break;
		}
	}

	if (pos >= 0)
		reserve_block(pos);

	return pos;
}

// Determine if inode is writable (Works for files and directories)
int is_writable(inode* node) {	
	return (node->flags % 3);
}

// Determine if inode is readonly (Works for files and directories)
int is_readonly(inode* node) {
	return !(node->flags % 3);
}

// Determine if inode is a directory
int is_dir(inode* node) {
	return (node->flags > 2);
}

// Determine if inode is a file
int is_file(inode* node) {
	return (node->flags < 2);
}

int is_valid_partition_flags(char flags) {
	return (0 <= flags && flags <= 1);
}

void* format(char* name, char flags, int num_blocks) {
	// Check input
	if (!(name != NULL &&
			num_blocks >= MIN_NUM_BLOCKS &&
			num_blocks <= MAX_NUM_BLOCKS &&
			is_valid_partition_flags(flags))) {
		#ifdef DEBUG
		printf("format: Invalid parameters\n");
		#endif
		return NULL;
	}

	partition = malloc(num_blocks * BLK_SIZE);

	inode root_node = init_inode("/", 4, 0);
	write_inode(root_node, 1);

	superblock new_superblock;
	strncpy(new_superblock.name, name, strlen(name));
	new_superblock.name[strlen(name)] = '\0';
	new_superblock.flags = flags;
	new_superblock.num_blocks = num_blocks;
	new_superblock.root_block = 1;
	new_superblock.num_free_blocks = num_blocks;
	memset(new_superblock.block_map, 0, sizeof(char)*756);
	write_super(new_superblock);
	reserve_block(0);
	reserve_block(1);

	// printf("%d\n", super->block_map[0]);
	// print_superblock(*super);

	return partition;
}

