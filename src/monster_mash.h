#include "standard_includes.h"
#include "server_includes.h"

#include<assert.h>
#include<sys/stat.h>
#include<time.h>
#include<stdarg.h>
#include<unistd.h>
#include<error.h>
#include<errno.h>


//Filesystem constants
#define INPUT_BUFFER_SIZE    128 
#define BLOCKS_PER_INODE     10
#define BLOCK_SIZE           4096
#define DISK_SIZE            104857600
#define INODE_TABLE_SIZE     8192 //in bytes, 1024 * INODE_SIZE
#define INODE_SIZE           8
#define FS_PATH              "../fs/mmash.fs"
#define MAX_FILENAME_LENGTH  32
#define MAX_OPEN_FILES       1024
#define METADATA_SIZE        (MAX_FILENAME_LENGTH + 4 + 2)
#define DIR_TABLE_ENTRY_SIZE (MAX_FILENAME_LENGTH + 4)
#define END                  -1

//Colors!
#define RED                  "\033[31m"
#define GREEN                "\033[32m"
#define YELLOW               "\033[33m"
#define BLUE                 "\033[34m"
#define MAGENTA              "\033[35m"
#define CYAN                 "\033[36m"
#define RESET                "\033[0m"

#define BOLDBLACK            "\033[1m\033[30m"
#define BOLDRED              "\033[1m\033[31m"
#define BOLDGREEN            "\033[1m\033[32m"
#define BOLDYELLOW           "\033[1m\033[33m"
#define BOLDBLUE             "\033[1m\033[34m"
#define BOLDMAGENTA          "\033[1m\033[35m"
#define BOLDCYAN             "\033[1m\033[36m"
#define BOLDWHITE            "\033[1m\033[37m"

//Globals, eww!
int force_printf; // For forcing read_data to printf instead of doing add_response.
//Functions
void get_local_input(char **user_input);
void get_remote_input(int socket, char **user_input);
void parse_input(char *input, int input_length);
void mkfs();
void make_dir(char* dir_name);
void ls();
void cd(char *dir_name);
void rmdir_mm(char *dir_name);
int  open_mm(char *file_flag);
void close_mm(char *fdt);
void close_by_name(char *name);
void write_mm(char *fd_text);
void seek_mm(char *fd_offset);
void read_mm(char *fd_size);
void link_mm(char *src_dest);
void unlink_mm(char *link_name);
void cat(char *file_name);
void import(char *input);
void export(char *input);
void wlog(char *format, ...);
void respond();
void add_to_response(char *format, ...);
void cp(char *src_dest);
void tree();
void stat_mm(char *name);
