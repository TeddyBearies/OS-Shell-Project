#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#define MAX_BUFFER 1024

int main() {
    char input[MAX_BUFFER];

    while (1) {
        //print
     char cwd[MAX_BUFFER];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s> ", cwd);
        } else {
          printf("MyShell> "); // fallback if getcwd fails
            }
        fflush(stdout);


        //read Input
        if (fgets(input, MAX_BUFFER, stdin) == NULL) {
            break; // Exit with ctrl d
        }
        input[strcspn(input, "\n")] = '\0';

       
        if (strcmp(input, "quit") == 0) {
            printf("quit successful\n");
            break;
        }
           

        //temporary test
        printf("You typed: %s\n", input);
    }
    return 0;
}