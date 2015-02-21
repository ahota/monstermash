#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<assert.h>
#include<sys/stat.h>
#include<time.h>

#define INPUT_BUFFER_SIZE 128 //Initial size, dynamic reallocation is used
#define BLOCKS_PER_INODE 10
#define BLOCK_SIZE 4096
#define DISK_SIZE 104857600
#define INODE_TABLE_SIZE 8192 //In terms of bytes = 1024 * INODE_SIZE(8)
#define INODE_SIZE 8
#define FS_PATH "../fs/mmash.fs"
#define MAX_FILENAME_LENGTH 32
#define MAX_OPEN_FILES 1024
#define METADATA_SIZE (MAX_FILENAME_LENGTH + 4 + 2)
#define DIR_TABLE_ENTRY_SIZE (MAX_FILENAME_LENGTH + 4)
#define END -1

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"

#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

//Function declarations
void parse_input(char *input, int input_length);
void mkfs();
void make_dir(char*);
void ls();
void cd(char*);
void rmdir(char*);
void open(char*);
void close(char *name);
void write(char*, int);
void seek(int, int);
void read(int, int);
void link(char*, char*);
void unlink(char*);
