#include <stdio.h>
#include <string.h>

int main() {
    char buffer[512];
    while (1) {
        printf(">"); //print input prompt
        strcpy(buffer, ""); //clean buffer from any prev input

        fgets(buffer, 511, stdin); //read input
        if (!strcmp(buffer, "exit")) { //if exit, terminate
            break;
        }
    }

    return 0; //return no errors
}