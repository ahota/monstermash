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
    /*
    FILE *disk = fopen(FS_PATH, "r");
    char root[2] = ".";
    if (disk != NULL) {
        cd(root);
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
    */

    //Let's mkfs for now
    mkfs();
    char root[2] = ".";
    cd(root);

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
        make_dir(strtok(NULL, "\n"));
    }
    else if(strcmp(command, "ls") == 0) {
        if(strtok(NULL, "\n") != NULL)
            fprintf(stderr, YELLOW "Ignoring arguments\n" RESET);
        ls();
    }
    else if(strcmp(command, "cd") == 0) {
        cd(strtok(NULL, " \n"));
    }
    else if(strcmp(command, "rmdir") == 0) {
        rmdir(strtok(NULL, " \n"));
    }
    else if(strcmp(command, "open") == 0) {
        //Send everything to open() and parse it there
        open(strtok(NULL, "\n"));
    }
    else if(strcmp(command, "close") == 0) {
        close(strtok(NULL, "\n"));
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
    else if(strcmp(command, "link") == 0) {
        link(strtok(NULL, " \n"), strtok(NULL, " \n"));
    }
    else if(strcmp(command, "unlink") == 0) {
        unlink(strtok(NULL, " \n"));
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
    //Check if name is NULL, empty, or contains a /
    if(name == NULL || strlen(name) == 0 || strchr(name, '/') != NULL) {
        fprintf(stderr, BOLDRED "Invalid directory name\n" RESET);
        return;
    }

    //Get any whitespace trimmed
    int start, end;
    trim_whitespace(name, &start, &end);
    //Name was all spaces
    if(end <= start) {
        fprintf(stderr, BOLDRED "Invalid directory name\n" RESET);
        return;
    }
    char *trimmed;
    //Check if argument starts with quotation mark
    if(name[start] == '"') {
        //Check that it ends with quotation mark
        if(name[end - 1] == '"') {
            //Name is too long
            if((end - 1) - (start + 1) > 32) {
                fprintf(stderr, BOLDRED "Directory name must be %d "
                        "or fewer characters long\n" RESET,
                        MAX_FILENAME_LENGTH);
                return;
            }
            trimmed = malloc((end - 1) - (start + 1) + 1);
            strncpy(trimmed, name + start + 1, (end - 1) - (start + 1));
            trimmed[(end - 1) - (start + 1)];
        }
        else {
            fprintf(stderr, BOLDRED "Mismatched quotation\n" RESET);
            return;
        }
    }
    else {
        trimmed = strtok(name, " \n");
        if(strtok(NULL, " \n") != NULL)
            fprintf(stderr, YELLOW "Ignoring arguments after space\n" RESET);
    }

    //Finally create the directory
    directory_create(trimmed, &inode_counter, &current_dir_inode);
}

void ls() {
    ls_dir(current_dir_inode);
}

void cd(char *name) {
    if(name == NULL || strlen(name) == 0) {
        fprintf(stderr, BOLDRED "Invalid argument\n" RESET);
        return;
    }
    current_dir_inode = ch_dir(name, &current_dir_inode);
    update_prompt(current_dir_inode, path);
}

void rmdir(char *name) {
    directory_remove(name, &current_dir_inode);
}

void open(char *file_flag) {
    //Input has filename (potentially with spaces) and flag
    
    printf("file_flag: %s\n", file_flag);

    char *name, *flag;
    smart_split(file_flag, name, flag);
    printf("open()\n");
    printf("name       = %s\n", name);
    printf("flag       = %s\n", flag);
    if(name == NULL || strlen(name) == 0) {
        fprintf(stderr, BOLDRED "Invalid file name\n" RESET);
        return;
    }
    if(flag == NULL || strlen(flag) == 0) {
        fprintf(stderr, BOLDRED "Must provide file access flag: "
                "'r', 'w', or 'rw'\n" RESET);
        return;
    }
    if(strcmp(flag, "r" ) != 0 &&
       strcmp(flag, "w" ) != 0 &&
       strcmp(flag, "rw") != 0) {
        fprintf(stderr, BOLDRED "Invalid flag. Must be one of 'r',"
                "'w', or 'rw'\n" RESET);
        return;
    }

    /*
    int start, end;
    trim_whitespace(file_flag, &start, &end);
    if(end <= start) {
        fprintf(stderr, BOLDRED "Invalid \n" RESET);
        return;
    }

    //end points to the character after the flag
    //name_end will point to the space just before the flag
    int name_end;
    for(name_end = end - 1; name_end > start; name_end--)
        if(file_flag[name_end] == ' ')
            break;

    printf("start: %d\nname_end: %d\nend: %d\n", start, name_end, end);

    char *flag = malloc(end - name_end);
    strncpy(flag, file_flag + name_end + 1, end - name_end - 1);
    flag[end - name_end - 1] = '\0';
    //Check if the flag is valid
    if(flag == NULL || strlen(flag) == 0) {
        fprintf(stderr, BOLDRED "Invalid flag. "
                "Must be 'r', 'w', or 'rw'\n" RESET);
        return;
    }

    //Trim the name
    for(; name_end > start; name_end--)
        if(file_flag[name_end] != ' ')
            break;
    name_end++;
    //Check if the name is too long or too short
    if(name_end - start > 32) {
        fprintf(stderr, BOLDRED "File name must be %d "
                "or fewer characters long\n" RESET, MAX_FILENAME_LENGTH);
        return;
    }
    if(name_end <= start) {
        fprintf(stderr, BOLDRED "Invalid name\n" RESET);
        return;
    }
    //name now has just the name
    char *name = malloc(name_end - start + 1);
    strncpy(name, file_flag + start, name_end - start);
    name[name_end - start] = '\0';

    printf("flag: %s\nname: |%s|\n", flag, name);
    */

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
            fprintf(stderr, BOLDRED "File does not exist.\n" RESET);
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
            fprintf(stderr, BOLDRED "File is already open with file "
                    "descriptor %d\n" RESET, fd);
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
    printf("Opened file `%s` with file descriptor %d\n", name, fd);
    return;
}

void close(char *name) {
    //name may have leading/trailing white space
    int start, end;
    trim_whitespace(name, &start, &end);
    char *trimmed = malloc(end - start + 1);
    strncpy(trimmed, name + start, end - start);
    trimmed[end - start] = '\0';

    int fd = file_exists(trimmed, &current_dir_inode);
    int i;
    for(i = 0; i < MAX_OPEN_FILES * 3; i += 3) {
        if (open_files[i] == fd) {
            open_files[i] = 0;
            open_files[i+1] = 0;
            open_files[i+2] = 0;
            n_open_files--;
            printf("Closed file %s (fd = %d)\n", trimmed, fd);
            return;
        }
    }
    fprintf(stderr, BOLDRED "File not found\n" RESET);
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
                fprintf(stderr, BOLDRED "Cannot write. File opened for "
                        "reading only. \n" RESET);
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
                fprintf(stderr, BOLDRED "Cannot read. File opened for "
                        "writing only. \n" RESET);
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

void link(char *dest, char *src) {
    //Check to see if file dest already exists 
    //Check to see if src exists
    //Create new link
    link_create(dest, src, &inode_counter, &current_dir_inode);
}

void unlink(char *name) {
    link_remove(name, &inode_counter, &current_dir_inode);
}
