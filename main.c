#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"

int main() {
    //char* path = getenv("PATH"); //currently not being used

    char buffer[512]; //stores input from user

    char* command[100]; //stores the tokenised input as an array of char pointers !! essential for execvp().
    
    while (1) {
        printf(">"); //print input prompt
        strcpy(buffer, ""); //clean buffer from any prev input

        for (int i = 0; i < 100; i++) {
            command[i] = NULL; //this should clear any tokens from the last loop 
        }

        if (fgets(buffer, 511, stdin) == NULL) { //read input and check for EOF (ctrl+d)
            printf("\n"); //print newline avoids segmentation fault
            break;
        }

        parse(buffer, command); //turn input into tokens

        execute(command); 
    }
    
    return 0; //return no errors
}

