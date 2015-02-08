#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>

#define INPUT_BUFFER_SIZE 1024
#define BLOCKS_PER_INODE 10
#define BLOCK_SIZE 4096
#define DISK_SIZE 104857600
#define INODE_TABLE_SIZE 1024 //In terms of inodes
#define INODE_SIZE 44
#define FS_PATH "../fs/mmash.fs"

typedef struct block {
    char type; //Either data or pointer to another block ('d' or 'p')
    void *data; //Array of pointers to either data or other blocks
}block;

typedef struct inode {
    char type; //file, directory or link
    short id; //The inode id
    block *block_pointers; //The last pointer may be an indirect block pointer
}inode;

typedef struct disk_s {
    inode *inode_table;
    void *data;
}disk_s;

//Function declarations
void parse_input(char *input, int input_length);
void mkfs();


