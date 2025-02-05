#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>

void parse(char* buffer, char** tokens);


int main() {
    char* path = getenv("PATH"); //currently not being used

    char buffer[512]; //stores input from user

    char* command[100]; //stores the tokenised input as an array of char pointers !! essential for execvp().
    
    while (1) {
        printf(">"); //print input prompt
        strcpy(buffer, ""); //clean buffer from any prev input

        for (int i = 0; i < 100; i++) {
            command[0] = NULL; //this should clear any tokens from the last loop 
        }

        if (fgets(buffer, 511, stdin) == NULL) { //read input and check for EOF (ctrl+d)
            printf("\n"); //print newline avoids segmentation fault
            break;
        }

        parse(buffer, command); //turn input into tokens

        if (!strcmp(command[0], "exit")) { //if exit, terminate
            break;
        }

        pid_t cpid = fork(); // child process created

        if (cpid < 0) {
            printf("Fork failed");
            return 1;
        }
        if (cpid == 0) {    // in the child process we created
            execvp(command[0], command); //this is the correct exec version, it takes a first argument for command, then arguments 
            perror(command[0]);
            exit(0);
        }
        else{ // in the parent process
            wait(NULL); // parent waits till child process is finished
            printf("Parent Proccess: Child has finished\n");
        }
    }

    return 0; //return no errors
}

void parse(char* buffer, char** tokens) {
    const char delims[8] = " \n><\t;&|";
    char* token = strtok(buffer, delims); 

    int i = 0;

    while (token != NULL) {
        tokens[i] = malloc(sizeof(char)*strlen(token)+1);
        strcpy(tokens[i], token);
        //printf("%s", tokens[i]);
        token = strtok(NULL, delims);
        ++i;
    }

    tokens[i] = NULL;
    /* TODO validate the input */
}