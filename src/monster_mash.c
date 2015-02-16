#include "monster_mash.h"

//Globals
short inode_counter = 0; //Bad global inode counter
int current_dir_inode = 0; //Offset of the current directory's inode
int n_open_files = 0;
char *prompt = "monster@test:";
char *path;
int *open_files;

int main() {
    printf("They did the monster mash!\n");
    char *user_input = malloc(INPUT_BUFFER_SIZE);
    path = malloc(MAX_FILENAME_LENGTH);
    open_files = malloc(MAX_OPEN_FILES * 3 * sizeof(int)); //Half of it is used for flags
    int i;
    for(i = 0; i < MAX_OPEN_FILES * 3; i++) {
        open_files[i] = 0;
    }

    // For persistency
    FILE *disk = fopen(FS_PATH, "r");
    if (disk != NULL) {
        cd(".");
        int i;
        for (i = 0; i < INODE_TABLE_SIZE; i += INODE_SIZE) {
            fseek(disk, i + 1, SEEK_SET);
            short inode_id = 0;
            fread(&inode_id, sizeof(short), 1, disk);
            if (inode_id != 0) {
                inode_counter++;
            }
            else {
                break;
            }
        }
        fclose(disk);
    }

    //Let's mkfs for now
    mkfs();

    while(1) {
        printf("%s%s $ ", prompt, path);
        fflush(NULL);
        fgets(user_input, INPUT_BUFFER_SIZE, stdin);
        int input_length = strlen(user_input);
        if(user_input[0] != '\n')
            parse_input(user_input, input_length);
    }
    return 0;
}

void parse_input(char *input, int input_length) {
    char *command, *input_copy;
    input_copy = strndup(input, input_length);
    command = strtok(input, " \n");
    int command_length = strlen(command);
    if (strcmp(command, "mkfs") == 0) {
        mkfs();
    }
    else if(strcmp(command, "mkdir") == 0) {
        //Send the next token of user input as an argument
        make_dir(strtok(NULL, " \n"));
    }
    else if(strcmp(command, "ls") == 0) {
        ls();
    }
    else if(strcmp(command, "cd") == 0) {
        cd(strtok(NULL, " \n"));
    }
    else if(strcmp(command, "rmdir") == 0) {
        rmdir(strtok(NULL, " \n"));
    }
    else if(strcmp(command, "open") == 0) {
        open(strtok(NULL, " \n"), strtok(NULL, " \n"));
    }
    else if(strcmp(command, "close") == 0) {
        close(strtok(NULL, " \n"));
    }
    else if(strcmp(command, "write") == 0) {
        write(strtok(NULL, " \n"), atoi(strtok(NULL, " \n")));
    }
    else if(strcmp(command, "seek") == 0) {
        seek(atoi(strtok(NULL, " \n")), atoi(strtok(NULL, " \n")));
    }
    else if(strcmp(command, "read") == 0) {
        read(atoi(strtok(NULL, " \n")), atoi(strtok(NULL, " \n")));
    }
    else if(strcmp(command, "exit") == 0) {
        printf("Bye!\n");
        exit(0);
    }
    else {
        printf("Invalid command: %s\n", command);
    }
}

void mkfs() {    
    mkdir("../fs", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    printf("Making filesystem...");
    current_dir_inode = disk_create(&inode_counter);
    update_prompt(current_dir_inode, path);
    printf("Done\n");
}

void make_dir(char *name) {
    directory_create(name, &inode_counter, &current_dir_inode);
}

void ls() {
    ls_dir(current_dir_inode);
}

void cd(char *name) {
    current_dir_inode = ch_dir(name, &current_dir_inode);
    update_prompt(current_dir_inode, path);
}

void rmdir(char *name) {
    directory_remove(name, &current_dir_inode);
}

void open(char *flag, char *name) {
    int fd = 0;
    /*
    Check if file exists
    If it doesn't:
        If reading: error
        If w or rw: 
            new file
            add it to the list of open files
            increment number of open files
    If it's already open:
        return file descriptor
        say file is already open
    If it exists:
        return file descriptor (inode_id)
        add it to the list of open files
        increment number of open files

    */
    fd = file_exists(name, &current_dir_inode);
    if (fd == -1) {
        if (strcmp(flag, "r") == 0) {
            fprintf(stderr, "File does not exist.\n");
            return;
        }
        else {
            fd = file_create(name, &inode_counter, &current_dir_inode);
        }
    }
    //Check if it's already open
    int i;
    for(i = 0; i < MAX_OPEN_FILES * 3; i += 3) {
        if (open_files[i] == fd) {
            fprintf(stderr, "File is already open with file descriptor %d!\n", fd);
            return;
        }
    }
    for(i = 0; i < MAX_OPEN_FILES * 3; i += 3) {
        if (open_files[i] == 0) {
            open_files[i] = fd;
            open_files[i+1] = (strcmp(flag, "r") == 0) ? 1 : ((strcmp(flag, "w") == 0) ? 2 : 3);
            open_files[i+2] = 0;
            n_open_files++;
        }
    }
    printf("Opened file %s with file descriptor %d\n", name, fd);
    return;
}

void close(char *name) {
    int fd = file_exists(name, &current_dir_inode);
    int i;
    for(i = 0; i < MAX_OPEN_FILES * 3; i += 3) {
        if (open_files[i] == fd) {
            open_files[i] = 0;
            open_files[i+1] = 0;
            open_files[i+2] = 0;
            n_open_files--;
        }
        else {
            fprintf(stderr, "File does not exist\n");   
        }
    }
}

void write(char *text, int fd) {
   /*
    Check to see if file is open
    If it's not:
        error
    else:
        Handle permissions
        write data
        move offset 
    */
    int i;
    for(i = 0; i < MAX_OPEN_FILES * 3; i += 3) {
        if (open_files[i] == fd) {
            if (open_files[i + 1] == 1) {
                fprintf(stderr, BOLDRED "Cannot write. File opened for reading only. \n" RESET);
                return;
            }
            else {
                write_data(fd, open_files[i+2], text);
                open_files[i+2] += strlen(text);
                return;
            }
        }
    }
    if (i == MAX_OPEN_FILES * 3) {
        fprintf(stderr, BOLDRED "Invalid file descriptor\n" RESET);
        return;
    }
       
}

void seek(int offset, int fd) {
    int i;
    for (i = 0; i < MAX_OPEN_FILES * 3; i += 3) {
        if (open_files[i] == fd) {
            open_files[i+2] = offset;
        }
    }
    if (i == MAX_OPEN_FILES) {
        fprintf(stderr, "File is not open.\n");
    }
}

void read(int size, int fd) {
    int i;
    for(i = 0; i < MAX_OPEN_FILES * 3; i += 3) {
        if (open_files[i] == fd) {
            if (open_files[i + 1] == 2) {
                fprintf(stderr, BOLDRED "Cannot read. File opened for writing only. \n" RESET);
                return;
            }
            else {
                read_data(fd, open_files[i+2], size);
                return;
            }
        }
    }
    if (i == MAX_OPEN_FILES * 3) {
        fprintf(stderr, BOLDRED "Invalid file descriptor\n" RESET);
        return;
    }   
}
