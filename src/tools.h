#include "monster_mash.h"

FILE *access_disk(int);
int commit_disk(FILE*);
short inode_create(char type, int, short*);
void block_create(char *, int);
short disk_create(short*);
short directory_create(char*, short*);
