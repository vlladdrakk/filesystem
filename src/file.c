#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "file.h"
#include "dir_utils.h"
#include "common.h"

superblock* super;
void* partition;

// returns node position
int find_node(char** name, int size){
	int pos = 0;
	int i = 0;
	int j = 0;
    inode* current_dir = read_inode(super->root_block);
    for (i = 0; i < size /*- 1*/ ; i++){
    	if (current_dir->flags == 4 ||  current_dir->flags == 3){ 
    		for(j=0; j < current_dir->file_size; j++){
                //if(check_block(current_dir->direct_refs[j])){
                    inode * tmp = read_inode(current_dir->direct_refs[j]);
                    if (strcmp(name[i],tmp->filename) == 0){
                        pos = current_dir->direct_refs[j];
                        current_dir = read_inode(pos);
                        break;
                    }    
                //}
    		}	
    	}else{
            break;
        }
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
	char* arr[MAX_DIRS];
    size_t size = str_split(name, arr, "/");
	char* file_name = arr[size-1];
	inode new_node = init_inode(file_name,flags,sz);
    int node_pos = alloc_block();
    // TO DO if filesize < size of refs ==> do not create data block
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
    	block* ptr = (block*)get_position_pointer(blk_pos);
		int j = 0;
    	for ( j = 0; j < cp_sz ; j++){
    		ptr->data[j] = fgetc(fp);
    	}
    }	
    write_inode(new_node, node_pos);

    int dir_pos = find_node(arr,size);
    fclose(fp);
	// Add node to directory
    add_to_directory(read_inode(dir_pos), node_pos);
    return SUCCESS;
}
int print_file(char* name){
	int n_chars = 0;
	// Search for file
	char* arr[MAX_DIRS];
    size_t size = str_split(name, arr, "/");
    int node_pos = find_node(arr,size);
    inode* node = read_inode(node_pos);
    if(check_block(node_pos) == 0 || is_file(node) == 0){
        return 0;
    }

    int i = 0;
    int j = 0;
	int blk_pos = 0;
    int num_blocks = node->file_size / BLK_SIZE + 1;
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
	char* arr[MAX_DIRS];
    size_t size = str_split(name, arr, "/");
    int node_pos = find_node(arr,size);
    inode* node = read_inode(node_pos);
    if(check_block(node_pos) == 0 || is_file(node) == 0){
        return FAILURE;
    }
    int i = 0;
	int blk_pos = 0;
    int num_blocks = node->file_size / BLK_SIZE + 1;
    for ( i = 0 ; i < num_blocks ; i++){
    	if ( i < 190){ 
    		blk_pos = node->direct_refs[i];
    	}else{
    		int* block = (int*)get_position_pointer(node->indirect_ref);
			blk_pos = block[i-190];
    	}
    	free_block(blk_pos);
    }
    int dir_pos = find_node(arr,size -1 );
    remove_from_directory(read_inode(dir_pos),node_pos);
    free_block(node_pos);
    return SUCCESS;
}