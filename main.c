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

        if (!strcmp(command, "exit")) { //if exit, terminate
            break;
        }
        /*
        pid_t cpid = fork(); // child process created

        if (cpid < 0) {
            printf("fork failed");
            return 1;
        }

        if (cpid == 0) {    // in the child process we created
            //execlp();
            exit(0);
        }
        else{ // in the parent process

            wait(NULL); // parent waits till child process is finished
            printf("Parent Proccess: Child has finished\n");
        }*/
        
    }

    return 0; //return no errors
}

char* parse(char* buffer) {
    char* tokens = strtok(buffer, " \n><\t;&"); //creates a pointer to tokens[0][0]
    //printf("%s", tokens);

    /* TODO validate the input */

    return tokens;
}