#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define INPUT_BUFFER_SIZE 1024

int main() {

    printf("They did the monster mash!\n");
    char *user_input = malloc(INPUT_BUFFER_SIZE);
    while(1) {
        printf("monster@butt:/$ ");
        fflush(NULL);
        fgets(user_input, INPUT_BUFFER_SIZE, stdin);
        int input_length = strlen(user_input);
        if(user_input[0] != '\n')
            printf("%s", user_input);
    }
    return 0;
}
