#include "monster_mash.h"

//Globals
short inode_counter = 0; //Used to generate inode IDs
int current_dir_inode = 0; //Offset of the current directory's inode
int n_open_files = 0;
char *prompt = "monster@test:";
char *path;
int *open_files;
int verbose = 1;
int server = 0;
char *server_greeting = "It was a graveyard smash!\n";

//socket file descriptors and port
int sock_fd, new_sock_fd, port;
//Length of client address
socklen_t client_length;
//input buffer from client
char buffer[INPUT_BUFFER_SIZE];
//server and client addresses
struct sockaddr_in server_address, client_address;

//Just for fun
char prompt_colors[][10] = {BOLDGREEN, BOLDYELLOW, BOLDBLUE, 
    BOLDMAGENTA, BOLDCYAN};

int main(int argc, char **argv) {
    //Check if user is starting as a server
    if(argc > 1) {
        if(strcmp(argv[1], "-s") == 0) {
            server = 1;
            if(argc > 2)
                port = atoi(argv[2]);
            else
                port = 1962; //year "Monster Mash" was released as a single
        }
    }

    //set up server stuff
    if(server) {
        //create a socket
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if(sock_fd < 0) {
            fprintf(stderr, BOLDRED "Could not open socket\n" RESET);
            return 1;
        }
        //zero out the address
        bzero((char *)&server_address, sizeof(server_address));

        //set up the address
        server_address.sin_family = AF_INET;
        server_address.sin_addr.s_addr = INADDR_ANY;
        server_address.sin_port = htons(port);

        //bind to socket
        int err;
        if((err = bind(sock_fd, (struct sockaddr *)&server_address,
           sizeof(server_address))) < 0) {
            fprintf(stderr, BOLDRED "Bind failed err=%d\n" RESET, err);
            return -1;
        }

        //listen to this socket
        listen(sock_fd, 5);
        client_length = sizeof(client_address);
        //block until we receive a connection
        new_sock_fd = accept(sock_fd, (struct sockaddr *)&client_address, 
                &client_length);
        if(new_sock_fd < 0) {
            fprintf(stderr, BOLDRED "Error accepting connection\n" RESET);
            return -1;
        }
        char *a = inet_ntoa(client_address.sin_addr);
        printf(YELLOW "Client connected %s\n" RESET, a);
    }


    if(server) {
        int n = write(new_sock_fd, server_greeting,
                strlen(server_greeting) + 1);
        if(n < strlen(server_greeting) + 1)
            fprintf(stderr, YELLOW "Warning: writing to client" 
                    "may have failed\n" RESET);
    }
    else
        printf("They did the Monster Mash!\n");

    path = malloc(MAX_FILENAME_LENGTH);

    //open_files structure:
    //[file descriptor] [read/write flag] [file offset]
    //File descriptor <=> inode_id
    open_files = malloc(MAX_OPEN_FILES * 3 * sizeof(int));
    int i;
    for(i = 0; i < MAX_OPEN_FILES * 3; i++) {
        open_files[i] = 0;
    }

    srand(time(NULL));

    //Let's mkfs for now
    mkfs();
    char root[2] = ".";
    cd(root);

    //If mmash was started as a server, it should wait until a user conencts
    //After connection:
    //  write prompt to socket
    //  read user input
    //  write output to socket

    while(1) {
        char *user_input = malloc(INPUT_BUFFER_SIZE);
    
        printf("%s%s%s%s $ ", prompt_colors[rand()%5], prompt, RESET, path);
        fflush(NULL);

        if(server)
            get_remote_input(new_sock_fd, &user_input);
        else
            get_local_input(&user_input);

        printf(YELLOW "DEBUG: %s" RESET, user_input);
        int input_length = strlen(user_input);
        if(input_length == 0) {
            printf(YELLOW "Client disconnected\n" RESET);
            break;
        }
        if(user_input[0] != '\n') {
            parse_input(user_input, input_length);
        }

        free(user_input);
    }
    return 0;
}

//Get user input from stdin
void get_local_input(char **user_input){
    int current_input_size = INPUT_BUFFER_SIZE;
    int c = EOF;
    int i = 0;
    while((c = getchar()) != '\n' && c != EOF) {
        (*user_input)[i++] = c;
        if (i == current_input_size) {
            //Reallocate more space
            current_input_size = i + INPUT_BUFFER_SIZE;
            *user_input = realloc(*user_input, current_input_size);
        }
    }            
    (*user_input)[i] = '\0';
}

//Get user input from the given socket
void get_remote_input(int socket, char **user_input) {
    bzero(*user_input, INPUT_BUFFER_SIZE);
    int current_input_size = INPUT_BUFFER_SIZE;
    int n = 0;
    int offset = 0;
    //block until we get input on this socket
    while((n = read(socket, *user_input + offset, current_input_size - 1)) == current_input_size) {
        current_input_size += INPUT_BUFFER_SIZE;
        *user_input = realloc(*user_input, current_input_size);
        offset += n;
    }
    wlog("N: %d\n", n);
    if(n < 0) {
        fprintf(stderr, BOLDRED "Error reading from client\n" RESET);
        return;
    }
    printf(YELLOW "CLIENT: %s" RESET, *user_input);
}

void parse_input(char *input, int input_length) {
    char *command, *input_copy;
    wlog("Input: %s\n", input);
    input_copy = strndup(input, input_length);
    command = strtok(input, " \n");
    int command_length = strlen(command);
    if (strcmp(command, "mkfs") == 0) { //                                  MKFS
        mkfs();
    }
    else if(strcmp(command, "mkdir") == 0) { //                            MKDIR
        make_dir(strtok(NULL, "\n"));
    }
    else if(strcmp(command, "ls") == 0) { //                                  LS
        if(strtok(NULL, "\n") != NULL)
            fprintf(stderr, YELLOW "Ignoring arguments\n" RESET);
        ls();
    }
    else if(strcmp(command, "cd") == 0) { //                                  CD
        cd(strtok(NULL, "\n"));
    }
    else if(strcmp(command, "rmdir") == 0) { //                            RMDIR
        rmdir(strtok(NULL, " \n"));
    }
    else if(strcmp(command, "open") == 0) { //                              OPEN
        int fd = open(strtok(NULL, "\n"));
        if(fd != -1)
            printf("Opened with file descriptor "GREEN"%d\n"RESET, fd);
    }
    else if(strcmp(command, "close") == 0) { //                            CLOSE
        close(strtok(NULL, "\n"));
    }
    else if(strcmp(command, "write") == 0) { //                            WRITE
        write(strtok(NULL, "\n"), atoi(strtok(NULL, " \n")));
    }
    else if(strcmp(command, "seek") == 0) { //                              SEEK
        seek_mm(atoi(strtok(NULL, " \n")), atoi(strtok(NULL, " \n")));
    }
    else if(strcmp(command, "read") == 0) { //                              READ
        read(atoi(strtok(NULL, " \n")), atoi(strtok(NULL, " \n")));
    }
    else if(strcmp(command, "link") == 0) { //                              LINK
        link(strtok(NULL, " \n"), strtok(NULL, " \n"));
    }
    else if(strcmp(command, "unlink") == 0) { //                          UNLINK
        unlink(strtok(NULL, " \n"));
    }
    else if(strcmp(command, "cat") == 0) { //                                CAT
        cat(strtok(NULL, "\n"));
    }
    else if(strcmp(command, "import") == 0) { //                          IMPORT
        import(strtok(NULL, "\n"));
    }
    else if(strcmp(command, "export") == 0) { //                          EXPORT
        export(strtok(NULL, " \n"), strtok(NULL, " \n"));
    }
    else if(strcmp(command, "cp") == 0) { //                                  CP
        cp(strtok(NULL, " \n"), strtok(NULL, " \n"));
    }
    else if(strcmp(command, "tree") == 0) { //                              TREE
        tree();
    }
    else if(strcmp(command, "stat") == 0) { //                              STAT
        stat_mm(strtok(NULL, "\n"));
    }
    else if(strcmp(command, "exit") == 0) { //                              EXIT
        if(server) {
            //Write to socket before closing
            close(sock_fd);
            close(new_sock_fd);
        }
        else
            printf("Bye!\n");

        exit(0);
    }
    else {
        if(server) //write to socket
            write(new_sock_fd, "Invalid command\n", 17);
        else
            printf("Invalid command: %s\n", command);
    }
}

void mkfs() {    
    mkdir("../fs", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    printf("Making filesystem...");
    current_dir_inode = disk_create(&inode_counter);
    update_prompt(current_dir_inode, path);
    printf(GREEN "Done\n" RESET);
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

            //This line used to be
            //trimmed[(end - 1) - (start + 1)];
            //"Here is a character"
            trimmed[(end - 1) - (start + 1)] = '\0';
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

    //Check if this directory already exists
    if(file_exists(trimmed, &current_dir_inode, 1) != -1) {
        fprintf(stderr, BOLDRED "Directory named `%s` already exists\n"
                RESET, name);
        return;
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

    //Check for quotation marks (user need not include them)
    if(name[0] == '"') {
        if(name[strlen(name) - 1] != '"') {
            fprintf(stderr, BOLDRED "Mismatched quotation\n" RESET);
            return;
        }
    }

    current_dir_inode = ch_dir(name, &current_dir_inode);
    update_prompt(current_dir_inode, path);
}

void rmdir(char *name) {
    directory_remove(name, &current_dir_inode);
}

int open_mm(char *file_flag) {
    //Input has filename (potentially with spaces) and flag
    char *name, *flag;
    smart_split(file_flag, &name, &flag);
    if(name == NULL || strlen(name) == 0) {
        fprintf(stderr, BOLDRED "Invalid file name\n" RESET);
        return -1;
    }
    if(flag == NULL || strlen(flag) == 0) {
        fprintf(stderr, BOLDRED "Must provide file access flag: "
                "'r', 'w', or 'rw'\n" RESET);
        return -1;
    }
    if(strcmp(flag, "r" ) != 0 &&
       strcmp(flag, "w" ) != 0 &&
       strcmp(flag, "rw") != 0) {
        fprintf(stderr, BOLDRED "Invalid flag. Must be one of 'r',"
                "'w', or 'rw'\n" RESET);
        return -1;
    }

    //Check if file exists
    int fd = 0;
    fd = file_exists(name, &current_dir_inode, 0);
    if (fd == -1) {
        if (strcmp(flag, "r") == 0) {
            fprintf(stderr, BOLDRED "File does not exist.\n" RESET);
            return -1;
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
            return fd;
        }
    }

    //Find the first free space in open files
    for(i = 0; i < MAX_OPEN_FILES * 3; i += 3) {
        if (open_files[i] == 0) {
            open_files[i]   = fd;
            open_files[i+1] = ((strcmp(flag, "r") == 0) ? 1 : //read
                              ((strcmp(flag, "w") == 0) ? 2 : //write
                              3));                            //read/write
            open_files[i+2] = 0;
            n_open_files++;
            break;
        }
    }

    //Check if we went all the way to the end of open_files
    if(i >= MAX_OPEN_FILES * 3) {
        fprintf(stderr, BOLDRED "Too many open files\n" RESET);
        return -1;
    }

    return fd;
}

void close_mm(char *name) {
    //name may have leading/trailing white space
    int start, end;
    char *trimmed;
    trim_whitespace(name, &start, &end);
    if(name[start] == '"') {
        if(name[end - 1] == '"') {
            trimmed = malloc((end - 1) - (start + 1) + 1);
            strncpy(trimmed, name + start + 1, (end - 1) - (start + 1));
            trimmed[(end - 1) - (start + 1)] = '\0';
        }
        else {
            fprintf(stderr, BOLDRED "Mismatched quotation\n" RESET);
            return;
        }
    }
    else {
        trimmed = strtok(name, " \n");
        if(strtok(NULL, " \n") != NULL)
            fprintf(stderr, BOLDYELLOW "Ignoring argument past space\n" RESET);
    }

    //Check if file exists
    int fd = file_exists(trimmed, &current_dir_inode, 0);
    if(fd == -1) {
        fprintf(stderr, BOLDRED "Invalid file argument\n" RESET);
        return;
    }

    //Look for file in open_files
    int i;
    wlog("fd = %d\n", fd);
    for(i = 0; i < MAX_OPEN_FILES * 3; i += 3) {
        if (open_files[i] == fd) {
            open_files[i] = 0;
            open_files[i+1] = 0;
            open_files[i+2] = 0;
            n_open_files--;
            wlog("Closed file %s (fd = %d)\n", trimmed, fd);
            return;
        }
    }
    fprintf(stderr, BOLDRED "File not found\n" RESET);
}

void write_mm(char *text, int fd) {
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
                open_files[i+2] += strlen(text); //Add to the file offset
                return;
            }
        }
    }
    if (i == MAX_OPEN_FILES * 3) {
        fprintf(stderr, BOLDRED "Invalid file descriptor\n" RESET);
        return;
    }
       
}

void seek_mm(int offset, int fd) {
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

void read_mm(int size, int fd) {
    if(fd == 0) {
        fprintf(stderr, BOLDRED "Invalid file descriptor\n" RESET);
        return;
    }
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

void cat(char *name) {
    int len = strlen(name);
    char *file_flag = malloc(strlen(name) + 3);
    strcpy(file_flag, name);
    file_flag[len] = ' ';
    file_flag[len + 1] = 'r';
    file_flag[len + 2] = '\n';
    int fd = open(file_flag);
    if (fd != -1) {
        read(DISK_SIZE, fd);
        close(name);    
    }
}

void import(char *input) {
    char *dest_name, *host_name;
    smart_split(input, &dest_name, &host_name);
    if(dest_name == NULL || strlen(dest_name) == 0) {
        fprintf(stderr, BOLDRED "Invalid destination file name\n" RESET);
        return;
    }
    if(host_name == NULL || strlen(host_name) == 0) {
        fprintf(stderr, BOLDRED "Invalid host file name\n" RESET);
        return;
    }
    if(strchr(dest_name, '/') != NULL) {
        fprintf(stderr, BOLDRED "Invalid destination file name\n" RESET);
        return;
    }

    int len = strlen(dest_name);
    char *file_flag = malloc(len + 3);

    strcpy(file_flag, dest_name);
    file_flag[len] = ' ';
    file_flag[len + 1] = 'w';
    file_flag[len + 2] = '\n';

    char buffer[1024];
    int fd = open(file_flag);
    if (fd != -1) {
        FILE *hfile = fopen(host_name, "r");
        while(fgets(buffer, sizeof(buffer), hfile) != NULL) {
            write(buffer, fd);
        }
        close(dest_name);
    }

}

void export(char *host_path, char *name) {
    FILE *temp_stdout = stdout;
    verbose = 0;
    stdout = fopen(host_path, "w+");
    cat(name);
    fclose(stdout);
    verbose = 1;
    stdout = temp_stdout;
}

void wlog(char *format, ...) {
    if (verbose) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

void cp(char *dest, char *src) {
    verbose = 0;
    int current_dir = current_dir_inode;
    int src_fd = find_inode_id(expand_path(src, &current_dir_inode, 0));
    char *dest_parent, *dest_name;
    get_parent_path(dest, &dest_parent);
    get_filename(dest, &dest_name);
    cd(dest_parent);

    //Open a file as destination
    int len = strlen(dest_name);
    char *file_flag = malloc(strlen(dest_name) + 3);
    strcpy(file_flag, dest_name);
    file_flag[len] = ' ';
    file_flag[len + 1] = 'w';
    file_flag[len + 2] = '\n';
    int dest_fd = open(file_flag);
    printf("%d-%d\n", src_fd, dest_fd);
    copy_data(src_fd, 0, DISK_SIZE, dest_fd);
    close(dest_name);
    current_dir_inode = current_dir;
    verbose = 1;
}

void tree() {
    print_tree(current_dir_inode, 0);
}

void stat_mm(char *name) {
    int start, end;
    trim_whitespace(name, &start, &end);
    
    if(end <= start) {
        fprintf(stderr, BOLDRED "Invalid file or directory name\n" RESET);
        return;
    }
    char *trimmed;
    if(name[start] == '"') {
        if(name[end - 1] == '"') {
            trimmed = malloc((end - 1) - (start + 1) + 1);
            strncpy(trimmed, name + start + 1, (end - 1) - (start + 1));
            trimmed[(end - 1) - (start + 1)] = '\0';
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

    //Find file/directory's inode
    //We want shallow = 1 so that if this is a link, we don't follow it
    int inode_id = file_exists(trimmed, &current_dir_inode, 1);
    if(inode_id == -1) {
        fprintf(stderr, BOLDRED "File/directory does not exist\n" RESET);
        return;
    }

    //If the user enters . , give them the actual name of the dir
    char *block_name;
    get_name((short)inode_id, &block_name);

    printf("Name on disk     | %s\n", block_name);
    printf("inode ID         | %d\n", inode_id);

    char type = inode_type((short)inode_id);
    printf("Type             | ");
    if(type == 'd')
        printf("directory\n");
    else if(type == 'f')
        printf("file\n");
    else if(type == 'l')
        printf("link\n");
    else
        printf("unknown\n");

    printf("Number of links  | %d\n",   get_link_count((short)inode_id));
    printf("Blocks allocated | %d\n",   block_count((short)inode_id));

    //Get total size of this file/dir
    float size = total_size((short)inode_id);
    char magnitude = 0; //pop pop!
    while(size > BLOCK_SIZE * 2) {
        size /= 1024;
        magnitude++;
    }

    //Smart print size
    printf("Total size       | ");
    if(magnitude == 0)
        printf("%d B\n", (int)size);
    else if(magnitude == 1)
        printf("%.1f kB\n", size);
    else if(magnitude == 2)
        printf("%.1f MB\n", size);
    else
        printf("%.1f ?B\n", size);

}














