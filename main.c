#include <stdio.h>
#include <string.h>

char* parse(char* buffer);

int main() {
    char buffer[512];
    while (1) {
        printf(">"); //print input prompt
        strcpy(buffer, ""); //clean buffer from any prev input

        fgets(buffer, 511, stdin); //read input

        char* command = parse(buffer);

        if (!strcmp(command, "exit")) { //if exit, terminate
            break;
        }
    }

    return 0; //return no errors
}

char* parse(char* buffer) {
    char* tokens = strtok(buffer, " "); //creates a pointer to tokens[0][0]
    printf(tokens);

    /* TODO validate the input */

    return tokens;
}