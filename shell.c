#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"

Alias* aliases[MAX_ALIASES] = {NULL};


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

    if (!strcmp(command[0], "alias")) {
        alias(command);
        return 1;
    }

    if (!strcmp(command[0], "unalias")) {
        unalias(command);
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
		if (chdir(command[1]) == -1) { // directory didnt change
			perror("cd");
		} else { // directory changed
			printf("successfully changed to %s \n", command[1]);
		}
	}
}

void alias(char** command) { //handles errors and arguments
    if (command[1] != NULL && command[2] == NULL) {
        printf("Incorrect number of arguments\n");
        return;
    }

    if (command[1] == NULL) {
        printAliases();
        return;
    }

    if (!addAlias(command)) {
        printf("Alias created\n");
        return;
    } else {
        printf("Error adding alias\n");
    }

    return;
}

/*
    Returns 0 for success, 1 for failure of any kind
*/

int addAlias(char** command) { //insert alias into array of Alias types
    int i = 0;
    while (i < MAX_ALIASES) {
        if (aliases[i] == NULL) {
            break;
        }
        ++i;
    }

    if (i == MAX_ALIASES) {
        printf("Alias max reached, ");
        return 1;
    }

    char* commandToAlias = malloc(255*sizeof(char) + 1); //allows arguments to be passed with the alias

    int j = 2;
    while(command[j] != NULL) {
        strcat(commandToAlias, command[j]); //add all arguments to the aliased command
        strcat(commandToAlias, " ");
        ++j;
    }

    aliases[i] = malloc(sizeof(Alias) + 1);

    aliases[i]->alias = command[1];
    aliases[i]->command = commandToAlias;
    return 0;
}

void unalias(char** command) { //handles errors and argument checking
    if (command[1] == NULL || command[2] != NULL) {
        printf("Incorrect number of arguments\n");
        return;
    }

    if (*aliases == NULL) {
        printf("No alias to remove\n");
        return;
    }

    if (!removeAlias(command)) {
        printf("Alias removed\n");
        return;
    } else {
        printf("Couldn't remove alias\n");
        return;
    }
}

/*
    Returns a 0 for success or a 1 for a failure of any kind
*/

int removeAlias(char** command) {
    int i = 0;
    while (i < MAX_ALIASES) {
        if (aliases[i] != NULL && !strcmp(aliases[i]->alias, command[1])) { //check if the alias exists in the list
            break;
        }
        ++i;
    }

    if (i == MAX_ALIASES) {
        printf("Alias not found, ");
        return 1;
    }

    free(aliases[i]);
    aliases[i] = NULL; //simply remove reference to alias and deallocate memory
    return 0;
}

void printAliases() {
    int empty = 1;
    for (int i = 0; i < MAX_ALIASES; i++) {
        if (aliases[i] != NULL) {
            printf("Alias: %s, Command: %s\n", aliases[i]->alias, aliases[i]->command); //just prints the alias and command for each extant alias
            empty = 0;
        }
    }
    if (empty) {
        printf("No aliases to print\n");
    }
}