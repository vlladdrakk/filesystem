#include "fs.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>

superblock* super;
void* partition;

void print_block(int* block) {
	int i;
	for (int i = 0; i < BLK_SIZE; i++) {
		printf("%d", block[i]);
	}
}

void print_superblock(superblock sblock) {
	printf("name: %s\nflags: %d\nroot_block: %d\nnum_free_blocks: %d\nblock_map %d-%d\n",
		sblock.name, sblock.flags, sblock.root_block, sblock.num_free_blocks, sblock.block_map[1],sblock.block_map[0]);
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
	new_inode.flags = flags;
	new_inode.file_size = file_size;
	memset(new_inode.direct_refs, 0, sizeof(int)*190);
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
	super->block_map[pos/8] |= 1 << pos % 8;
	super->num_free_blocks--;
}

void free_block(int pos) {
	super->block_map[pos/8] &= ~(1 << pos % 8);
	super->num_free_blocks++;
}

int get_bit(int pos) {
	return (super->block_map[pos/8] >> pos % 8) & 1;
}

int alloc_block() {
	int pos = -1;
	// Find free block
	int i;
	for (i = 0; i < super->num_blocks; i++) {
		if (get_bit(i) == 0) {
			pos = i;
			break;
		}
	}

	if (pos >= 0)
		reserve_block(pos);

	return pos;
}
// TO DO : check block size limit 
void add_to_directory(int directory_pos, int inode_pos) {
	inode* directory = read_inode(directory_pos);

	if (directory->file_size < 190) {
		// Use direct refs
		directory->direct_refs[directory->file_size] = inode_pos;
	} else {
		// Use indirect references
		if (directory->indirect_ref == 0)
			directory->indirect_ref = alloc_block();

		int* block = (int*)get_position_pointer(directory->indirect_ref);
		block[directory->file_size-190] = inode_pos;
	}

	directory->file_size++;
}

void remove_from_directory(int directory_pos,int inode_pos){
	// find reference to inode
	int i;
	int flag = 0;
	inode* directory = read_inode(directory_pos);
	for (i = 0; i<190 ; i++){
		if (directory->direct_refs[i] == inode_pos) {
			directory->direct_refs[i] = 0;
			free_block(inode_pos);
			flag = 1;
		}
	}
	if (!flag && directory->indirect_ref != 0){
		int block_index = -1;
		int* block = (int*)get_position_pointer(directory->indirect_ref);
		for (i = 0; i< directory->file_size - 190; i++){
			if (block[i] == inode_pos){
				block[i] = 0;
				free_block(inode_pos);
				block_index = i;
				// free whole block if no indirect_ref?

			}
		}
		if (block_index >= 0){
			// Put the last block_ref into empty block
			block[block_index] = block[directory->file_size - 190 - 1];
		}

	}
	directory->file_size --;
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

	printf("%d\n", super->block_map[0]);
	print_superblock(*super);

	return partition;
}

int main() {
	format("p1", 1, 1024);
	read_inode(super->root_block)->file_size = 190;
	int i,pos;
	for (i=0; i<7; i++) {
		inode n = init_inode("test"+i, 1, 10);
		pos = alloc_block();
		write_inode(n, pos);
		printf("adding %d to dir.\n",pos);
		add_to_directory(super->root_block, pos);
	}
	print_superblock(*super);
//	int* block = (int*)get_position_pointer(read_inode(super->root_block)->indirect_ref);
//	print_block(block);
	printf("removing from dir:\n");
	remove_from_directory(super->root_block,2);
	print_superblock(*super);
//	block = (int*)get_position_pointer(read_inode(super->root_block)->indirect_ref);
//	print_block(block);

}
