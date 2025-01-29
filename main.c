#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <unistd.h>
#include <sys/wait.h>

char* parse(char* buffer);

int main() {
    char buffer[512];
    while (1) {
        printf(">"); //print input prompt
        strcpy(buffer, ""); //clean buffer from any prev input

        if (fgets(buffer, 511, stdin) == NULL) { //read input and check for EOF (ctrl+d)
            printf("\n"); //print newline avoids segmentation fault
            break;
        }

        char* command = parse(buffer);

        if (!strcmp(command, "exit\n")) { //if exit, terminate
            break;
        }
        pid_t cpid = fork(); // child process created

        if (cpid == 0) {    // in the child process we created
            printf("child created successfully\n");
            exit(0); // terminate child process
        }
        else if (cpid > 0) { // in the parent process
            printf("Parent Process: Child Process ID is %d\n", cpid);
            wait(NULL); // parent waits till child process is finished
            printf("Parent Proccess: Child has finished\n");
        }
        else { // child process not created
            printf("Fork failed\n");
        }
    }

    return 0; //return no errors
}

char* parse(char* buffer) {
    char* tokens = strtok(buffer, " "); //creates a pointer to tokens[0][0]
    //printf("%s", tokens);

    /* TODO validate the input */

    return tokens;
}