#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"

void parse(char* buffer, char** tokens) {
    const char delims[8] = " \n><\t;&|";
    char* token = strtok(buffer, delims); 

    if (token == NULL) { //null terminate the input if the input is empty (segmentation fault prevention)
        tokens[0] = "\0";
        return;
    }

    int i = 0;

    while (token != NULL) {
        tokens[i] = malloc(sizeof(char) * strlen(token) + 1); //allocate memory for the tokens (+1 for \0)
        strcpy(tokens[i], token); //copy the token into the array of tokens
        token = strtok(NULL, delims); //get next token
        ++i;
    }

    tokens[i] = NULL; //null terminate the array of tokens so execvp doesn't error
}

void execute(char** command) {
    int status = checkSpecialCommands(command);

    if (status == 1) {
        return; //input empty flag, return to main without doing anything
    }

    pid_t cpid = fork(); // child process created

    if (cpid < 0) {
        printf("Fork failed");
        return;
    }
    if (cpid == 0) {    // in the child process we created
        execvp(command[0], command); //this is the correct exec version
        perror(command[0]);
        exit(0);
    }
    else{ // in the parent process
        wait(NULL); // parent waits till child process is finished
    }
}

int checkSpecialCommands(char** command) { 
    if (!strcmp(command[0], "exit")) { //if exit, terminate
        quick_exit(0); //exit code 0 no errors
    }

    if (!strcmp(command[0], "")) { //if the input is empty just reset the loop
        return 1; //return an integer to say "no input"
    }

    return 0; //return no special commands
}