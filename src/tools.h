#include "monster_mash.h"

FILE *access_disk(int);
int commit_disk(FILE*);
short inode_create(char*, char type,short*);
int block_create(char *);
int disk_create(short*);
int directory_create(char*, short*, int*);
void write_dir_data(char *name, int *current_dir_inode, short);
void update_prompt(int, char*);
void ls_dir(int);
int ch_dir(char*, int*);
int find_inode_offset(short);
short find_inode_id(int);
int directory_remove(char*, int*);
int file_exists(char*, int*);
int file_create(char*, short*, int*);
void write_data(int, int, char*);
void read_data(int, int, int);
void link_create(char*, char*, short*, int*);
char *get_parent_path(char *);
char *get_filename(char *);
void increment_link(int);
int decrement_link(int);
void link_remove(char *name, short *inode_counter, int *current_dir_inode); 
void remove_file_from_dir(FILE *disk, int parent_offset, int inode_id);
void trim_whitespace(char *name, int *start, int *end);
void smart_split(char *args, char *arg1, char *arg2);
