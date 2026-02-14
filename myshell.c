#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_BUFFER 1024

int main() {
    char input[MAX_BUFFER];

    while (1) {
        //print
        printf("MyShell> ");

        //read Input
        if (fgets(input, MAX_BUFFER, stdin) == NULL) {
            break; // Exit with ctrl d
        }

        //temporary test
        printf("You typed: %s", input);
    }
    return 0;
}./myshell
