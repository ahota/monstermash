#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define INPUT_BUFFER_SIZE 1024
#define NUM_BLOCK_POINTERS 10
#define BLOCK_SIZE 4096

void parse_input(char *input, int input_length);
void mkfs();

short inode_counter = 0; //Bad global inode counter

typedef struct block {
    char type; //Either data or pointer to another block ('d' or 'p')
    void *data; //Array of pointers to either data or other blocks
}block;

typedef struct inode {
    char type; //file, directory or link
    short id; //The inode id
    block *block_pointers; //The last pointer may be an indirect block pointer
}inode;
