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

void disk_create(disk_s *disk) {
    disk->inode_table = malloc(INODE_TABLE_SIZE * sizeof(inode*));
    int i;
    for (i = 0;i<INODE_TABLE_SIZE;i++) {
        disk->inode_table[i] = *inode_create('u');//Undeclared
    }
    //disk->data = malloc(DISK_SIZE - INODE_TABLE_SIZE * INODE_SIZE);
}

void commit_disk(disk_s *disk) {
    FILE *file = fopen(FS_PATH, "wb");
    if (file != NULL) {
        //Write inode_table to file
        printf("not null, writing\n");
        int i, j;
        for (i = 0;i < INODE_TABLE_SIZE; i++) {
            printf("%d\n", i);
            fwrite(&(disk->inode_table[i].type), sizeof(char), 1, file);
            fwrite(&(disk->inode_table[i].id), sizeof(short), 1, file);
            for (j = 0; j < BLOCKS_PER_INODE; j++) {
                fwrite(disk->inode_table[i].block_pointers[j].data, BLOCK_SIZE, 1, file); 
            }
        }
        fclose(file);
        printf("Done writing\n");
    }
}


