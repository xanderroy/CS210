#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"

char* originalPath;

int main() {
    originalPath = getenv("PATH"); //store original path

    char* home = getenv("HOME"); //store home directory

    chdir(home); //go to home directory before anything else

    loadAliases();

    load_history();

    char buffer[512]; //stores input from user

    char* command[100]; //stores the tokenised input as an array of char pointers !! essential for execvp().
    
    while (1) {
        printf(">"); //print input prompt
        strcpy(buffer, ""); //clean buffer from any prev input

        for (int i = 0; i < 100; i++) {
            command[i] = "\0"; //this should clear any tokens from the last loop 
        }

        if (fgets(buffer, 511, stdin) == NULL) { //read input and check for EOF (ctrl+d)
            printf("\n"); //print newline avoids segmentation fault
            save_history(); 
            returnPath();
            saveAliases();
            break;
        }

        parse(buffer, command); //turn input into tokens
        if (command[0][0] != '\0' && command[0][0] != '!' && strcmp(command[0], "clearhistory") != 0 ){
        	history_add(command);
    	}
        execute(command); 
        
    }
    
    return 0; //return no errors
}

void returnPath() {
    setenv("PATH", originalPath, 1);
    printf("%s\n", getenv("PATH")); //test that restoring path works (it does)
}
