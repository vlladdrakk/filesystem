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

void print_partition() {
	int x = 15;
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
// TO DO : check block size limit
void add_to_directory(inode* directory, int inode_pos) {
	if (directory->file_size < MAX_DREFS) {
		// Use direct refs
		directory->direct_refs[directory->file_size] = inode_pos;
	} else {
		// Use indirect references
		if (directory->indirect_ref == 0)
			directory->indirect_ref = alloc_block();

		int* block = (int*)get_position_pointer(directory->indirect_ref);
		block[directory->file_size-MAX_DREFS] = inode_pos;
	}

	directory->file_size++;
}

void remove_from_directory(int directory_pos,int inode_pos) {
	// find reference to inode
	int i;
	int flag = 0;
	inode* directory = read_inode(directory_pos);
	for (i = 0; i<MAX_DREFS ; i++){
		if (directory->direct_refs[i] == inode_pos) {
			directory->direct_refs[i] = 0;
			free_block(inode_pos);
			flag = 1;
		}
	}
	if (!flag && directory->indirect_ref != 0){
		int block_index = -1;
		int* block = (int*)get_position_pointer(directory->indirect_ref);
		for (i = 0; i< directory->file_size - MAX_DREFS; i++){
			if (block[i] == inode_pos){
				block[i] = 0;
				free_block(inode_pos);
				block_index = i;
				// free whole block if no indirect_ref?

			}
		}
		if (block_index >= 0){
			// Put the last block_ref into empty block
			block[block_index] = block[directory->file_size - MAX_DREFS - 1];
		}

	}
	directory->file_size --;
}

inode* get_child(inode* dir, char* child) {
	// Check in direct references
	int direct_limit = dir->file_size > MAX_DREFS ? MAX_DREFS : dir->file_size;
	int i;
	for (i = 0; i < direct_limit; i++) {
		if (dir->direct_refs[i] == 0)
			break;

		inode* temp = read_inode(dir->direct_refs[i]);
		if (strcmp(temp->filename, child) == 0)
			return temp;
	}

	return NULL;
}

int mkdir(char* name, char flags) {
	// validate path
	char** directories = strsplit("/etc/var/etc/", "/");
	inode* current_dir, *child;
	int valid_dir = 1;

	current_dir = read_inode(super->root_block);
	int i = 0;
	while (directories[i] != NULL) {
		// printf("%d: %s\n", i, directories[i]);
		if ((child = get_child(current_dir, directories[i])) != NULL) {
			printf("/%s\n", directories[i]);
			current_dir = child;
		} else {
			valid_dir = 0;
			break;
		}
		i++;
	}

	// Get parent inode

	// Run checks on parent

	// create directory inode

	// Attach new directory inode

	return valid_dir;
}

void* format(char* name, char flags, int num_blocks) {
	partition = malloc(num_blocks * BLK_SIZE);

	inode root_node = init_inode("/", 4, 0);
	write_inode(root_node, 1);

	superblock new_superblock;
	strncpy(new_superblock.name, name, strlen(name));
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
