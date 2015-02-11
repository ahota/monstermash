#include "monster_mash.h"

FILE *access_disk(int);
int commit_disk(FILE*);
short inode_create(char*, char type,short*);
int block_create(char *);
int disk_create(short*);
int directory_create(char*, short*);
void update_prompt(int, char*);
