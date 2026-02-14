#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#define MAX_BUFFER 1024
void handle_cd(char *input) {
    // input looks like: "cd" or "cd foldername"
    char *path = input + 2; // move past "cd"

    // skip spaces after cd
    while (*path == ' ') path++;

    if (*path == '\0') {
        // just "cd" -> print current directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
        return;
    }

    // try to change directory
    if (chdir(path) != 0) {
        perror("cd");
        return;
    }

    // update PWD environment variable after successful chdir
    char newCwd[1024];
    if (getcwd(newCwd, sizeof(newCwd)) != NULL) {
        setenv("PWD", newCwd, 1);
    }
}

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

       // quit
        if (strcmp(input, "quit") == 0) {
            printf("quit successful\n");
            break;
        }
           //cd
        if (strncmp(input, "cd", 2) == 0 && (input[2] == '\0' || input[2] == ' ')) {
            handle_cd(input);
            continue; // skips the (you typed)line
        }   


        //temporary test
        printf("You typed: %s\n", input);
    }
    return 0;
}