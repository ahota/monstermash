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
