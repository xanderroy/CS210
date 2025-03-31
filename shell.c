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

  // First check if the command is an alias
  if (command[0] != NULL && strcmp(command[0], "") != 0) { // check if first token is NULL or empty string
    int i = 0;
    while (i < MAX_ALIASES) { // Loop through all aliases up to MAX_ALIASES
      if (aliases[i] != NULL && !strcmp(aliases[i]->alias, command[0])) { // Check an alias exists at the index then check if the name matches the first token of the command

        // Matching alias found
        char buffer[1024] = {0}; // Initialise a buffer with zeros
        strcpy(buffer, aliases[i]->command); // Copy the alias command into the buffer

        // Add additional arguments
        int j = 1;
        while (command[j] != NULL) { // Iterate through all arguments left
          strcat(buffer, " "); // Add a space between arguments
          strcat(buffer, command[j]); // Add the next argument
          j++;
        }

        // Free the command tokens before parsing allocates new memory
        for (int a = 0; command[a] != NULL; a++) {
          free(command[a]); // Free allocated memory to the command tokens
          command[a] = NULL; // Set pointers to NULL
        }

        // Parse new command
        parse(buffer, command);
        break;
      }
      i++;
    }
  }

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

        saveAliases();

        save_history();

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
    if (!strcmp(command[0], "history")){
    	if(command[1] != NULL){
    		printf("Please enter only one parameter\n");
    	}else{
    	history_print();
    	}
    	return 1;
    }

    if (command[0][0] == '!')  { // history invocation started
        if (command[1] != NULL) {
            printf("Incorrect number of arguments, usage !<no> OR !-<no> OR !!.\n");
            return 1;
        }
    	if (!strcmp(command[0], "!!")) { // re-execute last entered command
    		if (history_size > 0) {
        		int last_index = (history_index - 1 + 20) % 20; // go to previous index 
        		execute(history[last_index]);
        		return 1;
    		} else {
        		printf("No commands in history.\n");
                return 1;
    		}
		}
	else if (command[0][1] != '-'  && isdigit(command[0][1])) { // the handling of the !<no> case
            	int number = atoi(command[0] + 1);  // Convert from string to integer
		 if (number > 0 && number <= history_size) {
		 	int index = (history_index - history_size + number - 1 + 20) % 20; 
        		execute(history[index]);
        		return 1;
   		    } else if (number > 20) {
                printf("Number is greater than maximum history size.\n");
                return 1;
            } else if (number > history_size){
        		printf("Number is greater than current size.\n");
        		return 1;
    		} else {
       			printf("Usage: !<no> OR !-<no>\n");
       			return 1;
       		}
        	}
         else if (command[0][1] == '-') { // the handling of the !-<no> case
         	if(isdigit(command[0][2])){
            		int number = atoi(command[0] + 2);  // convert from string to integer
            if (number > 20) {
                printf("Number greater than maximum history size.\n");
                return 1;
            }
			if (number > 0 && number <= history_size) {
        			int index = (history_index - number + 20) % 20; 
        			execute(history[index]);
        			return 1;
   			} else {
        			printf("No command found.\n");
        			return 1;
    			}
       		 }else{
       			printf("Usage: !<no> OR !-<no>\n");
       			return 1;
       		}
        }else{
        	printf("Usage: !<no> OR !-<no>\n");
        	return 1;
        }
	}
	if (strcmp(command[0], "clearhistory") == 0) {
    		delete_history();
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
        printf("Incorrect number of arguments, usage: cd OR cd PATH\n");
        return;
    }
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

	} else { // if second argument passed
		if (chdir(command[1]) == -1) { // directory didnt change
			perror(command[1]);
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
        if (!strcmp(command[1], aliases[i]->alias)) {
            printf("Alias already taken, please unalias and retry.\n");
            return 1;
        }
        ++i;
    }

    if (i == MAX_ALIASES) {
        printf("Alias max reached.\n");
        return 1;
    }

    char* commandToAlias = malloc(1024); //allows arguments to be passed with the alias
    strcpy(commandToAlias, "");
    
    int j = 2;
    while(command[j] != NULL) {
        strcat(commandToAlias, command[j]); //add all arguments to the aliased command
        strcat(commandToAlias, " ");
        ++j;
    }

    aliases[i] = malloc(sizeof(Alias));
    aliases[i]->command = malloc(1024);
    aliases[i]->command = "\0";

    aliases[i]->alias = strdup(command[1]);
    aliases[i]->command = strdup(commandToAlias);
    free(commandToAlias);
    return 0;
}

void unalias(char** command) { //handles errors and argument checking
    if (command[1] == NULL || command[2] != NULL) {
        printf("Incorrect number of arguments\n");
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
    
    if (aliasesEmpty()) {
        printf("Aliases list is empty, ");
        return 1;
    }

    if (i == MAX_ALIASES) {
        printf("Alias not found, ");
        return 1;
    }

    free(aliases[i]);
    aliases[i] = NULL; //simply remove reference to alias and deallocate memory
    return 0;
}

int aliasesEmpty() {
    int found = 1;
    for (int i = 0; i < MAX_ALIASES; ++i) {
        if (aliases[i] == NULL) {
            continue;
        }
        if (aliases[i] != NULL) {
            found = 0;
        }
    }
    return found;
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
    return;
}

void loadAliases() {
    FILE* file = fopen(".aliases", "r"); 
    
    if (file == NULL) { //if the file doesn't exist, do this
        //perror("Error loading aliases");
        return;
    }

    char *rb = malloc(100*sizeof(char));

    int i = 0;

    char* alias[100]; //must be as long as the max alias length permitted earlier to handle long aliases.

    while (fgets(rb, 99, file)) { //get the read buffer from file, then parse it as if it's a command.
        parse(rb, alias);
        if (!strcmp(alias[0], "alias")) {
            addAlias(alias); //just add alias to the array using the old method.
        } else {
            printf("Error in .aliases file: Incorrect Format. Did not load aliases.\n");
            return;
        }
        ++i;
    }

    free(rb);
    fclose(file);

    return;

}

//when saving aliases, save the alias command to the file. That way, you can add the alias using the addAlias method later

void saveAliases() {
    chdir(getenv("HOME")); //go to home dir to save file

    FILE* file = fopen(".aliases", "w"); 

    if (file == NULL) { //if file cannot be opened for whatever reason
        perror("Could not save aliases");
        return;
    }

    int i = 0;
    
    char *wb = malloc(100*sizeof(char));
    
    while(i != 10 && aliases[i] != NULL) {
        strcpy(wb, "\0"); //clear old write buffer

        strcpy(wb, "alias "); //copy alias command 

        strcat(wb, aliases[i]->alias); //add alias 

        strcat(wb, " "); //add space for parsing

        strcat(wb, aliases[i]->command); //add aliased command

        strcat(wb, "\0"); //null terminate write buffer

        fputs(wb, file); //write the buffer to file

        fputs("\n", file); //new line

        ++i;
    }

    free(wb);

    fclose(file);
    return;
}

void history_add(char** command){
	char** full_command = malloc(sizeof(char*) * 100);
	int i = 0;
	while (command[i] != NULL) {
        	full_command[i] = strdup(command[i]);
        	i++;
    		}
    	full_command[i] = NULL; // make last character null.
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
    int start = (history_index - history_size + 20) % 20; 
    for (int i = 0; i < history_size; i++) {
        int index = (start + i) % 20;  
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
    //reset variables 
    history_size = 0;	
    history_index = 0;  
    printf("History has been cleared\n");
} 

void save_history() {
    chdir(getenv("HOME")); //go to home dir to save file

    FILE* file = fopen(".hist_list", "w"); 

    if(!file) {
        perror("Error opening history file for writing");
        return;
    }

    int index = history_index - history_size + 20; // Start from oldest command
    for (int i = 0; i < history_size; i++) {
        int pos = (index + i) % 20;
        if (history[pos] != NULL) {
            for (int j = 0; history[pos][j] != NULL; j++) {
                fprintf(file, "%s ", history[pos][j]);
            }
            fprintf(file, "\n");
        }
    }
    fclose(file);
}

void load_history() {
    FILE * file = fopen(".hist_list", "r");
    if (!file) {
        return;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char *tokens[100] = {NULL};
        parse(line, tokens);
        history_add(tokens);
    }
    fclose(file);
}
