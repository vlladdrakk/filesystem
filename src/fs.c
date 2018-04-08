#include "fs.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>

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
	printf("name: %s\nflags: %d\nfile_size: %d\ndirect_refs: %d\nindirect_refs %d-%d\n",
		node.filename, node.flags, node.file_size, node.direct_refs, node.indirect_ref);	
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
	new_inode.filename[strlen(name)+1] = 0;
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

void remove_from_directory(int directory_pos,int inode_pos) {
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
// returns node position
int find_node(char** name, int size){
	int pos = 0;
	int i = 0;
	int j = 0;
    inode* current_dir = read_inode(super->root_block);
    for (i = 0; i < size /*- 1*/ ; i++){
    	if (current_dir->flags == 4 ||  current_dir->flags == 3){ 
    		for(j=0; j < current_dir->file_size; j++){
    			//printf("reading inode from direct_ref %d\n",j);
    			inode * tmp = read_inode(current_dir->direct_refs[j]);
    			//printf("tmp->filename = %s, name[i] is %s\n",tmp->filename,name[i]);
    			if (/*tmp->flags == 4 && */strcmp(name[i],tmp->filename) == 0){
    				pos = current_dir->direct_refs[j];
    				current_dir = read_inode(pos);
    				break;
    			}
    		}	
    	}/*else{ //its a file
			if (strcmp(name[i],current_dir->filename) == 0){
				//printf("find_node:its a file\n");
				break;
			}
    	}*/
    }
	return pos;
}

int copy_file(char* name, char flags, char* local_file){
	int num_blocks;
	FILE* fp = fopen(local_file,"r");
	if( fp == 0){
		printf("Could Not Open The File.\n");
		exit(1);
	}
	// Create new node for file
	fseek(fp, 0, SEEK_END);
	int sz = ftell(fp);
    rewind(fp);
	//char** arr = strsplit(name,"/");
	char* arr[MAX_DIR];
    size_t size = str_split(name, arr, "/");
	char* file_name = arr[size - 1];
	inode new_node = init_inode(file_name,flags,sz);
    int node_pos = alloc_block();
    // TO DO if filesize < size of refs ==> do not create data block
   //  if ( sz < 190 ) {
   //  	printf("sz < 190. writing to direct refs\n");
   //  	for( i = 0; i < sz; i++)
   //  		new_node.direct_refs[i] = fgetc(fp);
   //  } else if ( sz < 190 + BLK_SIZE){
   //  	if (new_node.indirect_ref  == 0){
			// new_node.indirect_ref = alloc_block();
			// block* ptr = (block*)get_position_pointer(new_node.indirect_ref);
			// for ( j = 0; j < sz ; j++){
	  //   		ptr->data[j] = fgetc(fp);
	  //   	}
   //  	}
   //  }
   //  else{
	// Create data block
	// Copy data to data block
    num_blocks = sz / BLK_SIZE + 1;
    if ( super->num_free_blocks < num_blocks ){
    	return FAILURE;
    }
	int i = 0;
    for ( i = 0 ; i < num_blocks ; i++){
    	int blk_pos = alloc_block();
    	// Add data block to inode
    	if ( i < 190){ // Add to direct_refs
    		new_node.direct_refs[i] = blk_pos;
    	}else{ // Add to indirect_refs
    		if (new_node.indirect_ref  == 0)
    			new_node.indirect_ref = alloc_block();
    		int* block = (int*)get_position_pointer(new_node.indirect_ref);
			block[i-190] = blk_pos;

    	}
    	int cp_sz = BLK_SIZE;
    	if (BLK_SIZE * (i+1) > sz){
    		// copy the remaining amount, not whole block
    		cp_sz = sz % BLK_SIZE;
    	}
    	//printf("block position = %d\n",blk_pos);
    	block* ptr = (block*)get_position_pointer(blk_pos);
		int j = 0;
    	for ( j = 0; j < cp_sz ; j++){
    		ptr->data[j] = fgetc(fp);
    	}
    }	
	    	
    //}
    write_inode(new_node, node_pos);
    int dir_pos = find_node(arr,size);
    fclose(fp);
	// Add node to directory
    add_to_directory(dir_pos, node_pos);

    return SUCCESS;
}
int print_file(char* name){
	//printf("\n\nprinting file %s\n",name);
	int n_chars = 0;
	// Search for file
	char* arr[MAX_DIR];
    size_t size = str_split(name, arr, "/");
    int node_pos = find_node(arr,size);
    if (node_pos == 0){
    	return FAILURE;
    }
    inode* node = read_inode(node_pos);
    int i = 0;
    int j = 0;
	int blk_pos = 0;
    int num_blocks = node->file_size / BLK_SIZE + 1;
    /*if(node->file_size < 190 ){
    	// read from direct refs
    	printf("node->file_size < 190. reading from direct refs\n");
    	for(i = 0; i < node->file_size; i++){
    		printf("%d",i);
    		printf("%c",node->direct_refs[i]);
    	}

    } else if( node->file_size < 190 + BLK_SIZE){
    	// read from indirect refs
    }else{*/
	// read from blocks
	for ( i = 0 ; i < num_blocks ; i++){
    	if ( i < 190){ // Read from direct blocks
    		blk_pos = node->direct_refs[i];
    	}else{
    		int* block = (int*)get_position_pointer(node->indirect_ref);
			blk_pos = block[i-190];
    	}
    	int cp_sz = BLK_SIZE;
    	if (BLK_SIZE * (i+1) > node->file_size){ // print the remaining amount, not whole block
    		cp_sz = node->file_size % BLK_SIZE;
    	}
    	block* ptr = (block*)get_position_pointer(blk_pos);
    	for ( j = 0; j < cp_sz ; j++){
    		printf("%c",ptr->data[j]);
    		n_chars++;
    	}
    }
    //}
    
	return n_chars;
}

int remove_file(char* name){
	char* arr[MAX_DIR];
    size_t size = str_split(name, arr, "/");
    int node_pos = find_node(arr,size);
    if (node_pos == 0){
    	return FAILURE;
    }
    inode* node = read_inode(node_pos);
    int i = 0;
    int j = 0;
	int blk_pos = 0;
    int num_blocks = node->file_size / BLK_SIZE + 1;
    for ( i = 0 ; i < num_blocks ; i++){
    	if ( i < 190){ 
    		blk_pos = node->direct_refs[i];
    	}else{
    		int* block = (int*)get_position_pointer(node->indirect_ref);
			blk_pos = block[i-190];
    	}
    	int cp_sz = BLK_SIZE;
    	if (BLK_SIZE * (i+1) > node->file_size){ 
    		cp_sz = node->file_size % BLK_SIZE;
    	}
    	free_block(blk_pos);
    }
    int dir_pos = find_node(arr,size-1);
    remove_from_directory(dir_pos,node_pos);
    free_block(node_pos);
    return SUCCESS;
}
