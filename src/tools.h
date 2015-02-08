#include "monster_mash.h"

inode *inode_create(char type);
block *block_create(char type);
void disk_create(disk_s*);
void commit_disk(disk_s*);
