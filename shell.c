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
        return; //return to main without doing anything
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
        returnPath();

        quick_exit(0); //exit code 0 no errors
    }

    if (!strcmp(command[0], "")) { //if the input is empty just reset the loop
        return 1; //return 1 to mean nothing further to do
    }

    if (!strcmp(command[0], "getpath")) {
        getpath(command);
        return 1;
    }

    if (!strcmp(command[0], "setpath")) {
        setpath(command);
        return 1;
    }
    
    if (!strcmp(command[0], "cd")) {
        cd(command);  
        return 1;
    }

    return 0; //return no special commands
}

void getpath(char** command) {
    if (command[1] != NULL) {
        printf("Incorrect number of arguments, usage: getpath\n"); //error
        return;
    }

    printf("%s\n", getenv("PATH")); //print current value of the PATH
    return; 
}

void setpath(char** command) {
    if (command[2] != NULL || command[1] == NULL) {
        printf("Incorrect number of arguments, usage: setpath PATH\n"); //error
        return;
    }

    setenv("PATH", command[1], 1); //overwrite value of path with given argument
    return;
}

void cd(char** command){
    if (command[2] != NULL) {
        printf("Incorrect number of arguments\n");
        return;
    }

	if(command[1] == NULL){
		char* home = getenv("HOME"); //store home directory
		if(home == NULL){
			perror("cd failure\n");
		} 
		if(chdir(home) == -1){  // if directory hasnt changed to home
			perror("cd failed to go home\ncd");
		} else { // directory changed
			printf("directory successfully changed to home\n");
		}
	} else { // if second argument passed
		if(chdir(command[1]) == -1){ // directory didnt change
			perror("cd");
		} else { // directory changed
			printf("successfully changed to %s \n", command[1]);
		}
	}
}

