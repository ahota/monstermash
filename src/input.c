#include "monster_mash.h"


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

