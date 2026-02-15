#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>


extern char **environ;

#define MAX_BUFFER 1024


void handle_cd(char *input) {
    // input "cd" or "cd foldername"
    char *path = input + 2; // move past "cd"

    // skip spaces after cd
    while (*path == ' ') path++;

    if (*path == '\0') {
        // if just "cd" then print current directory
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

    // update PWD variable after chdir
    char newCwd[1024];
    if (getcwd(newCwd, sizeof(newCwd)) != NULL) {
        setenv("PWD", newCwd, 1);
    }
}

void handle_dir(char *input) {
    // input "dir" or "dir folder"
    char *folder = input + 3; // move past "dir"

    while (*folder == ' ') folder++; // skip spaces

    if (*folder == '\0') {
        folder = "."; // if no folder given the go to current directory
    }

    DIR *d = opendir(folder);
    if (d == NULL) {
        perror("dir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        // print every entry name
        printf("%s\n", entry->d_name);
    }

    closedir(d);
}

void handle_environ() {
    char **env = environ;

    while (*env != NULL) {
        printf("%s\n", *env);
        env++;
    }
}

void handle_set(char *input) {
    
    char var[128];
    char value[512];

    // try to read "set <var> <value>"
    if (sscanf(input, "set %127s %511[^\n]", var, value) != 2) {
        printf("Usage: set VARIABLE VALUE\n");
        return;
    }

    if (setenv(var, value, 1) != 0) {
        perror("set");
    }
}

void handle_pause() {
    printf("PAUSED - Press Enter to unpause:");
    fflush(stdout);

    // read until newline
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // do nothing
    }
}

void handle_echo(char *input) {
    // input looks like: "echo something here"
    char *msg = input + 4; // move past "echo"

    while (*msg == ' ' || *msg == '\t') msg++; // skip spaces/tabs after echo

    // print message with space/tab cleanup
    int lastWasSpace = 0;

    while (*msg != '\0') {
        if (*msg == ' ' || *msg == '\t') {
            if (!lastWasSpace) {
                putchar(' ');
                lastWasSpace = 1;
            }
        } else {
            putchar(*msg);
            lastWasSpace = 0;
        }
        msg++;
    }

    putchar('\n');
}

//open help.txt with more
void handle_help() {
    system("more help.txt");
}

int make_args(char *input, char *argv[]) {
    int argc = 0;

    argv[argc] = strtok(input, " \t");
    while (argv[argc] != NULL && argc < 63) {
        argc++;
        argv[argc] = strtok(NULL, " \t");
    }

    return argc;
}

void run_external(char *input) {
    char *argv[64];
    pid_t pid;
    int status;

    int argc = make_args(input, argv);
    if (argc == 0) return; // empty line

    pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        // child runs the program
        execvp(argv[0], argv);
        perror("execvp"); // only runs if exec fails
        exit(1);
    } else {
        // parent waits so we don't create zombies
        waitpid(pid, &status, 0);
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
        //dir
        if (strncmp(input, "dir", 3) == 0 && (input[3] == '\0' || input[3] == ' ')) {
            handle_dir(input);
            continue;
        }   
        //environ
        if (strcmp(input, "environ") == 0) {
            handle_environ();
            continue;
        }

        // set
        if (strncmp(input, "set", 3) == 0 && (input[3] == '\0' || input[3] == ' ')) {
        handle_set(input);
        continue;
        }
        //pause
    if (strcmp(input, "pause") == 0) {
        handle_pause();
        continue;
        }

        //echo
    if (strncmp(input, "echo", 4) == 0 && (input[4] == '\0' || input[4] == ' ' || input[4] == '\t')) {
        handle_echo(input);
        continue;
        }
        //help
    if (strcmp(input, "help") == 0) {
        handle_help();
        continue;
}

        //run as external command
        run_external(input);


        //temporary test
        printf("You typed: %s\n", input);
    }
    
    return 0;
}