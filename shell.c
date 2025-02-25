#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"
#include <ctype.h>

char** history[20];
int history_size = 0;
int history_index = 0;

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
    if (!strcmp(command[0], "history")){
    	if(command[1] != NULL){
    		printf("Please enter only one parameter\n");
    	}else{
    	history_print();
    	}
    	return 1;
    }

    if (command[0][0] == '!')  { // history invocation started
    	if (!strcmp(command[0], "!!")) { // re-execute last entered command
    		if (history_size > 0) {
        		int last_index = (history_index - 1 + 20) % 20; // go to previous index 
        		execute(history[last_index]);
        		return 1;
    		} else {
        		printf("No commands in history.\n");
    			}
		}
	else if (command[0][1] != '-'  && isdigit(command[0][1])) { // the handling of the !<no> case
            	int number = atoi(command[0] + 1);  // Convert from string to integer
		 if (number > 0 && number <= history_size) {
		 	int index = (history_index - history_size + number - 1 + 20) % 20; 
        		execute(history[index]);
        		return 1;
   		} else if (number > history_size){
        		printf("number is greater than size.\n");
        		return 1;
    		}else{
       			printf("Please enter an integer\n");
       			return 1;
       		}
        	}
         else if (command[0][1] == '-') { // the handling of the !-<no> case
         	if(isdigit(command[0][2])){
            		int number = atoi(command[0] + 2);  // convert from string to integer
			 if (number > 0 && number <= history_size) {
        			int index = (history_index - number + 20) % 20; 
        			execute(history[index]);
        			return 1;
   			} else {
        			printf("No command found.\n");
        			return 1;
    			}
       		 }else{
       			printf("Please enter an integer\n");
       			return 1;
       		}
        }else{
        	printf("Please enter an integer\n");
        	return 1;
        }
        if (strcmp(command[0], "clearhistory") == 0) {
    		delete_history();
    		return 1;
	}
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
	if(command[1] == NULL){
		char* home = getenv("HOME"); //store home directory
		if(home == NULL){
			perror("cd failure\n");
		} 
		if(chdir(home) == -1){  // if directory hasnt changed to home
			perror("cd failed to go home\n");
		}else{ // directory changed
			printf("directory successfully changed to home\n");
		}
	}else{ // if second argument passed
		if(chdir(command[1]) == -1){ // directory didnt change
			perror("cd failed to change to the specified directory\n");
		}else{ // directory changed
			printf("successfully changed to %s \n", command[1]);
		}
	}
}


void history_add(char** command){
	char** full_command = malloc(sizeof(char*) * 100);
	int i = 0;
	while (command[i] != NULL) {
        	full_command[i] = strdup(command[i]);
        	i++;
    		}
    	full_command[i] = NULL; // add terminator at the end
	if (history[history_index] != NULL) {
        	for (int j = 0; history[history_index][j] != NULL; j++) {
            		free(history[history_index][j]);  // free each token of index after wrap around occurs
        	}
        free(history[history_index]); // free index
    	}
	history[history_index] = full_command;
	history_index = history_index + 1; 
	history_index = history_index % 20; // this is to wrap around
	if(history_size < 20){
		history_size++;  
	}
}

void history_print() {
    if (history_size == 0) {
        printf("No commands stored in history.\n");
        return;
    }
    int recent = history_index;
    for (int i = 0; i < history_size; i++) {
        int index = (recent -1 - i + 20) % 20; 
        if (history[index] != NULL) {
            printf("%d: ", i + 1);
            for (int j = 0; history[index][j] != NULL; j++) {
                printf("%s ", history[index][j]);  
            }
            printf("\n");
        }
    }
}

void delete_history() {
    for (int i = 0; i < history_size; i++) {
        if (history[i] != NULL) {
            for (int j = 0; history[i][j] != NULL; j++) {
                free(history[i][j]); 
            }
            free(history[i]);
            history[i] = NULL; 
        }
    }
    //reset variables below
    history_size = 0;	
    history_index = 0;  
}







