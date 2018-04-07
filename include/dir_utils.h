#ifndef DIR_UTILS_H
#define DIR_UTILS_H

int is_valid_dir_flags(char flags);

int validate_path(char* absolute_path);

inode* get_parent_dir(char* absolute_path);

char* get_dir_name(char* absolute_path);

void remove_from_directory(int directory_pos,int inode_pos);

void add_to_directory(inode* directory, int inode_pos);

int mkdir(char* name, char flags);

int rmdir(char* name);

#endif