#include "monster_mash.h"

inode *inode_create(char type) {
    inode *result = malloc(sizeof(inode*));
    result->type = type;
    result->id = inode_counter++;
    result->block_pointers = malloc(NUM_BLOCK_POINTERS * sizeof(block*));   
    return result;
}

block *block_create(char type) {
    block *result = malloc(sizeof(block*));
    result->type = type;
    result->data = malloc(BLOCK_SIZE);
    return result;
}
