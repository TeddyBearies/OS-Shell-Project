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
        // remove the newline at the end (so comparisons work)
        input[strcspn(input, "\n")] = '\0';

        // built-in: quit
        if (strcmp(input, "quit") == 0) {
            printf("quit successful\n");
            break;
        }
           

        //temporary test
        printf("You typed: %s\n", input);
    }
    return 0;
}