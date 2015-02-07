#include "monster_mash.h"

int main() {

    printf("They did the monster mash!\n");
    char *user_input = malloc(INPUT_BUFFER_SIZE);
    while(1) {
        printf("monster@butt:/$ ");
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
    else {
        printf("Invalid command: %s\n", command);
    }
}

void mkfs() {
    printf("Making filesystem...\n");
}
