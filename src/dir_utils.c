#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "fs.h"
#include "common.h"
#include "dir_utils.h"

extern superblock* super;
extern void* partition;

// Returns an array of inode pointers referencing each inode that is a
// child of the directory
inode** get_children(inode* dir) {
	// Create child list
	inode** child_list = malloc(sizeof(inode*));
	int list_size = 0;

	// Check in direct references
	int direct_limit = dir->file_size > MAX_DREFS ? MAX_DREFS : dir->file_size;
	int i;
	for (i = 0; i < direct_limit; i++) {
		if (dir->direct_refs[i] == 0)
			break;

		// Increase list size
		child_list = realloc(child_list, sizeof(inode*) * ++list_size);
		child_list[list_size - 1] = read_inode(dir->direct_refs[i]);
	}

	if (dir->file_size > MAX_DREFS) {
		// Check indirect refs
		int* refs = get_position_pointer(dir->indirect_ref);
		int i = 0;
		while (refs[i] != 0) {
			// Increase list size
			child_list = realloc(child_list, sizeof(inode*) * ++list_size);
			child_list[list_size - 1] = read_inode(refs[i]);

			i++;
		}
	}

	return child_list;
}

// Prints the contents of the directory in a format similar to `ls -l`
void print_dir(inode* dir) {
	inode** children = get_children(dir);
	printf("Flags\tSize\tName\n");
	int i;
	for (i = 0; i < dir->file_size; i++) {
		inode* child = children[i];
		printf("%d\t%d\t%s\n", child->flags, child->file_size, child->filename);
	}

	free(children);
}

int ls(char* path) {
	// validate path
	int valid_dir = validate_path(path);

	if (valid_dir == FAILURE)
		return FAILURE;

	// Get parent inode
	inode* parent = get_parent_dir(path);

	print_dir(parent);

	return SUCCESS;
}

inode* get_child(inode* dir, char* child) {
	if (dir->file_size == 0)
		return NULL;

	if (child == NULL)
		return NULL;

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

	if (dir->file_size > MAX_DREFS) {
		// Check indirect refs
		int* refs = get_position_pointer(dir->indirect_ref);
		int i = 0;
		while (refs[i] != 0) {
			inode* temp = read_inode(refs[i]);
			if (strcmp(temp->filename, child))
				return temp;

			i++;
		}
	}

	return NULL;
}

int mkdir(char* name, char flags) {
	// Check input
	if (!(name != NULL &&
			is_valid_dir_flags(flags))) {
		#ifdef DEBUG
		printf("mkdir: Invalid parameters\n");
		#endif
		return FAILURE;
	}

	// validate path
	int valid_dir = validate_path(name);

	if (valid_dir == FAILURE)
		return FAILURE;

	// Get parent inode
	inode* parent = get_parent_dir(name);

	// Run checks on parent
	// Check if writable
	if (parent->flags != 4) {
		#ifdef DEBUG
		printf("mkdir: Parent directory not writable\n");
		#endif
		return FAILURE;
	}

	// Check that the directory is not full
	if (parent->file_size == MAX_DIRS) {
		#ifdef DEBUG
		printf("mkdir: directory is full\n");
		#endif
		return FAILURE;
	}

	// Get the name of the new directory
	char* dir_name = get_dir_name(name);

	// create directory inode
	inode new_dir = init_inode(dir_name, flags, 0);
	int dir_block = alloc_block();

	if (dir_block == -1) {
		#ifdef DEBUG
		printf("mkdir: Cannot allocate a block\n");
		#endif
		return FAILURE;
	}

	write_inode(new_dir, dir_block);
	free(dir_name);

	// Attach new directory inode
	add_to_directory(parent, dir_block);

	return SUCCESS;
}

int rmdir(char* name) {
	// Check input
	if (name == NULL) {
		#ifdef DEBUG
		printf("rmdir: Invalid parameters\n");
		#endif
		return FAILURE;
	}

	// validate path
	int valid_dir = validate_path(name);

	if (valid_dir == FAILURE)
		return FAILURE;

	// get parent node
	inode* parent = get_parent_dir(name);

	// get target directory
	char* dir_name = get_dir_name(name);
	inode* target_dir = get_child(parent, dir_name);
	free(dir_name);

	// Run checks on directory
	if (!is_dir(target_dir)) {
		#ifdef DEBUG
		printf("rmdir: Path provided is not a directory\n");
		#endif
		return FAILURE; // inode isn't a directory
	}

	if (target_dir->file_size > 0) {
		#ifdef DEBUG
		printf("rmdir: Directory is not empty\n");
		#endif
		return FAILURE; // Can't delete a non-empty directory
	}

	if (target_dir->flags != D_RW) {
		#ifdef DEBUG
		printf("rmdir: Provided directory is read only\n");
		#endif
		return FAILURE; // directory is not writable
	}

	// remove directory from parent
	int parent_pos = get_inode_pos(parent);
	int target_pos = get_inode_pos(target_dir);
	remove_from_directory(parent_pos, target_pos);
	free_block(target_pos);

	return SUCCESS;
}

// Check that given flags are valid for a directory
int is_valid_dir_flags(char flags) {
	return (3 <= flags && flags <= 4);
}

// Determine if the given absolute path is valid
int validate_path(char* absolute_path) {
	char** path = strsplit(absolute_path, "/");
	inode* current_dir, *child;

	// Return success for root directory
	if (path[0] == NULL)
		return SUCCESS;

	current_dir = read_inode(super->root_block);
	int i = 0;
	while (path[i+1] != NULL) { // Loop while there is another directory
		if ((child = get_child(current_dir, path[i])) != NULL) {
			// Ensure the child is a directory
			if (!is_dir(child)) {
				#ifdef DEBUG
				printf("mkdir: Directory is a file\n");
				#endif
				return FAILURE;
			}

			// check the permissions
			if (is_writable(child)) {
				current_dir = child;
			} else {
				#ifdef DEBUG
				printf("mkdir: Reached read only directory\n");
				#endif
				return FAILURE;
			}
		} else {
			#ifdef DEBUG
			printf("mkdir: Invalid directory provided\n");
			#endif
			return FAILURE;
		}
		i++;
	}

	free(path); // Free the string array

	return SUCCESS;
}

// Gets the parent of the directory in the given path.
// This function assumes the directory is valid.
inode* get_parent_dir(char* absolute_path) {
	char** path = strsplit(absolute_path, "/");
	inode* current_dir;

	// Handle getting root directory
	if (path[0] == NULL)
		return read_inode(super->root_block);

	current_dir = read_inode(super->root_block);
	int i = 0;
	while (path[i+1] != NULL) { // Loop while there is another directory
		current_dir = get_child(current_dir, path[i]);
		i++;
	}
	
	return current_dir;
}

char* get_dir_name(char* absolute_path) {
	char** path = strsplit(absolute_path, "/");
	
	// Loop through the string array to get to the end
	int i = 0;
	while(path[i+1] != NULL)
		i++;

	// Copy the directory name
	char* copied_str = malloc(strlen(path[i]) + 1);
	strncpy(copied_str, path[i], strlen(path[i]));
	copied_str[strlen(path[i])] = '\0';

	free(path); // Free the string array

	return copied_str;
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

void test_dirs() {
	// Need to have a partition
	if (partition == NULL)
		format("testing_dirs", 1, 64);

	// creating directories
	puts("Test: Creating directories in root...");
	int etc_ret = mkdir("/etc", D_RO);
	int var_ret = mkdir("/var", D_RW);
	int home_ret = mkdir("/home", D_RW);

	if (etc_ret == FAILURE)
		puts("Failed to create read only directory!");

	if (var_ret == FAILURE || home_ret == FAILURE)
		puts("Failed to create writable directories!");

	// Creating subdirectories
	puts("\nTest: Creating subdirectories");
	int conf_ret = mkdir("/etc/conf", D_RW); // this should fail because /etc is read only
	int log_ret = mkdir("/var/log", D_RW);

	if (conf_ret == SUCCESS)
		puts("Failed! Was able to create subdirectory in read only directory.");

	if (log_ret == FAILURE)
		puts("Failed to create subdirectory!");

	// removing directories
	puts("\nTest: Removing directories");
	
	int rm_var = rmdir("/var"); // Should fail because it isn't empty
	int rm_etc = rmdir("/etc"); // Should fail because it is read only
	int rm_home = rmdir("/home"); // Should pass
	int rm_log = rmdir("/var/log"); // Should pass

	if (rm_var == SUCCESS)
		puts("Failed! Removed non-empty directory!");

	if (rm_etc == SUCCESS)
		puts("Failed! Removed read only directory!");

	if (rm_home == FAILURE)
		puts("Failed to remove empty directory!");

	if (rm_log == FAILURE)
		puts("Failed to remove empty subdirectory!");
}
