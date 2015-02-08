#include "monster_mash.h"

inode *inode_create(char type);
block *block_create(char type);
void disk_create(char*, short*);
void commit_disk(char*);
