#include "tools.h"

inode *inode_create(char type) {
    inode *result = malloc(sizeof(inode*));
    result->type = type;
    result->id = 0;//inode_counter++;
    result->block_pointers = malloc(BLOCKS_PER_INODE * sizeof(block*));   
    return result;
}

block *block_create(char type) {
    block *result = malloc(sizeof(block*));
    result->type = type;
    result->data = malloc(BLOCK_SIZE);
    return result;
}

void disk_create(char *disk, short *inode_counter) {
    //Build root inode
    disk[0] = 'd';
    int block_offset = INODE_TABLE_SIZE * INODE_SIZE;
    int cur_inode = (*inode_counter)++;
    disk[1] = block_offset >> 24;
    disk[2] = block_offset >> 16;
    disk[3] = block_offset >> 8;
    disk[4] = block_offset >> 0;
    disk[5] = cur_inode >> 8;
    disk[6] = cur_inode >> 0;
    disk[7] = 0;
    
    //Build root directory
    
}

void commit_disk(char *disk) {
    FILE *file = fopen(FS_PATH, "wb");
    if (file != NULL) {
        fwrite(disk, sizeof(char), DISK_SIZE, file);
        fclose(file);
        printf("Done writing\n");
    }
}


