#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<sys/stat.h>

#define INPUT_BUFFER_SIZE 1024
#define BLOCKS_PER_INODE 10
#define BLOCK_SIZE 4096
#define DISK_SIZE 104857600
#define INODE_TABLE_SIZE 8192 //In terms of bytes = 1024 * INODE_SIZE(8)
#define INODE_SIZE 8
#define FS_PATH "../fs/mmash.fs"
#define MAX_FILENAME_LENGTH 32

typedef struct block {
    char type; //Either data or pointer to another block ('d' or 'p')
    void *data; //Array of pointers to either data or other blocks
}block;

typedef struct inode {
    char type; //file, directory or link
    short id; //The inode id
    int first_block_offset; //The offset for the first block of the file/directory
}inode;

typedef struct disk_s {
    inode *inode_table;
    void *data;
}disk_s;

//Function declarations
void parse_input(char *input, int input_length);
void mkfs();
void make_dir(char*);
void ls();
void cd(char*);

