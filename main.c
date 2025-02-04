#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>

char* parse(char* buffer);


int main() {
    char* path = getenv("PATH");

    char buffer[512];
    
    while (1) {
        printf(">"); //print input prompt
        strcpy(buffer, ""); //clean buffer from any prev input

        if (fgets(buffer, 511, stdin) == NULL) { //read input and check for EOF (ctrl+d)
            printf("\n"); //print newline avoids segmentation fault
            break;
        }

        char* command[100]; 
        *command= parse(buffer);

        if (!strcmp(command[0], "exit")) { //if exit, terminate
            break;
        }

        pid_t cpid = fork(); // child process created

        if (cpid < 0) {
            printf("Fork failed");
            return 1;
        }
        if (cpid == 0) {    // in the child process we created
            execvp(command[0], command);
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

char* parse(char* buffer) {
    char* tokens[100];
    const char delims[8] = " \n><\t;&|";
    char* token = strtok(buffer, delims); 

    for (int i = 0; token != NULL; i++) {
        tokens[i] = token;
        token = strtok(NULL, delims);
    }

    /* TODO validate the input */

    return *tokens;
}