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

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

//Function declarations
void parse_input(char *input, int input_length);
void mkfs();
void make_dir(char*);
void ls();
void cd(char*);
void rmdir(char*);
