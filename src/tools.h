#include "monster_mash.h"

// Disk functions
FILE * access_disk(int access_type);
int    commit_disk(FILE *disk);
int    disk_create(short *inode_counter);
void   wipe(FILE* disk, int offset, int n_bytes);

//Inode functions
short  inode_create(char *name, char type, short *inode_counter);
int    find_inode_offset(short inode_id);
short  find_inode_id(int inode_offset);
char   inode_type(short inode_id);

//Block functions
int    block_create(char *name);

//Directory functions
int    directory_create(char *name, short *inode_counter, int *current_inode);
void   write_dir_data(char *name, int *current_dir_inode, short inode_id);
void   ls_dir(int current_dir_inode);
int    ch_dir(char *name, int *current_dir_inode);
int    directory_remove(char *name, int *current_dir_inode);
void   remove_file_from_dir(FILE *disk, int parent_offset, int inode_id);

//File functions
int    file_exists(char *name, int *current_dir_inode, int shallow);
int    file_create(char *name, short *inode_counter, int *current_dir_inode);
void   write_data(int fd, int file_offset, char *text);
void   read_data(int fd, int file_offset, int size);
void   link_create(char *name, char *src, 
        short *inode_counter, int *current_inode);
void   get_parent_path(char *path, char **ret);
void   get_filename(char *path, char **ret);
void   increment_link(int target_inode_offset);
int    decrement_link(int target_inode_offset);
void   link_remove(char *name, short *inode_counter, int *current_dir_inode);
void   copy_data(int fd, int file_offset, int size, int dest_fd);
void   print_tree(int current_dir_inode, int depth);
void   print_space(int num, int corner);

//Utilities
void   update_prompt(int current_dir_inode, char *path);
int    expand_path(char *path, int *current_dir_inode, int shallow); 
int    insert_entry(int block_offset, char *name, short inode_id);
short  find_entry(int block_offset, char *name);
int    remove_entry(int block_offset, char *name);
void   trim_whitespace(char *name, int *start, int *end);
void   smart_split(char *args, char **arg1, char **arg2);
