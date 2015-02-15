#include "monster_mash.h"

//Globals
short inode_counter = 0; //Bad global inode counter
int current_dir_inode = 0; //Offset of the current directory's inode
char *prompt = "monster@test:";
char *path;

int main() {

    printf("They did the monster mash!\n");
    char *user_input = malloc(INPUT_BUFFER_SIZE);
    path = malloc(MAX_FILENAME_LENGTH);
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
    else if(strcmp(command, "exit") == 0) {
        //Need to delete all our crap
        //Pass
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
