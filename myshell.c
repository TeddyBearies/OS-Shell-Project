#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>



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

int check_background(char *input) {
    int len = strlen(input);

    // remove spaces
    while (len > 0 && (input[len - 1] == ' ' || input[len - 1] == '\t')) {
        input[len - 1] = '\0';
        len--;
    }

    // check for &
    if (len > 0 && input[len - 1] == '&') {
        input[len - 1] = '\0'; // remove &
        
        // remove spaces again after removing &
        len--;
        while (len > 0 && (input[len - 1] == ' ' || input[len - 1] == '\t')) {
            input[len - 1] = '\0';
            len--;
        }

        return 1; // background
    }

    return 0; // not background
}

int setup_output_redirect(char *input) {
    // returns a file descriptor for stdout, or -1 if no redirect
    char *pos = strstr(input, ">>");
    int append = 0;

    if (pos != NULL) {
        append = 1;
    } else {
        pos = strchr(input, '>');
        append = 0;
    }

    if (pos == NULL) return -1;

    // split the string at > or >>
    *pos = '\0';
    if (append) pos++; // move past first '>'
    pos++;             // move past second '>' if append, or the only '>' if not

    while (*pos == ' ' || *pos == '\t') pos++; // skip spaces

    if (*pos == '\0') {
        printf("Redirection error: missing output file\n");
        return -1;
    }

    int flags = O_CREAT | O_WRONLY;
    if (append) flags |= O_APPEND;
    else flags |= O_TRUNC;

    int fd = open(pos, flags, 0644);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    return fd;
}

int setup_input_redirect(char *input) {
    // returns fd for stdin, or -1 if no redirect
    char *pos = strchr(input, '<');
    if (pos == NULL) return -1;

    *pos = '\0'; // cut command part
    pos++;       // move past '<'

    while (*pos == ' ' || *pos == '\t') pos++; // skip spaces

    if (*pos == '\0') {
        printf("Redirection error: missing input file\n");
        return -1;
    }

    int fd = open(pos, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    return fd;
}


void run_external(char *input) {
    char *argv[64];
    pid_t pid;
    int status;

    int background = check_background(input);

    // check if there is > or >>
    int out_fd = setup_output_redirect(input);
    int in_fd = setup_input_redirect(input);
    
    int argc = make_args(input, argv);
    if (argc == 0) return; // empty line

    pid = fork();

    if (pid < 0) {
        perror("fork");
        if (out_fd != -1) close(out_fd);
        return;
    }

    if (pid == 0) {
        // child runs the program

        if (out_fd != -1) {
            dup2(out_fd, 1); // redirect stdout
            close(out_fd);
        }

        if (in_fd != -1) {
            dup2(in_fd, 0); // redirect stdin
            close(in_fd);
        }

        execvp(argv[0], argv);
        perror("execvp");
        exit(1);
    } else {
        // parent closes its copy
        if (out_fd != -1) {
            close(out_fd);
        }
        if (in_fd != -1) {
            close(in_fd);
        }

        // parent waits only if not background
        if (!background) {
            waitpid(pid, &status, 0);
        } else {
            printf("[background pid %d]\n", pid);
        }
    }
}









int main(int argc, char *argv[]) {

        FILE *in = stdin;
    int interactive = 1;

    if (argc == 2) {
        in = fopen(argv[1], "r");
        if (in == NULL) {
            perror("batch file");
        return 1;
    }
    interactive = 0;
} else if (argc > 2) {
    printf("Usage: %s [batchfile]\n", argv[0]);
    return 1;
}

    char input[MAX_BUFFER];

    while (1) {
     if (interactive) {

        //print
     char cwd[MAX_BUFFER];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
    printf("%s> ", cwd);
        } else {
          printf("MyShell> "); // fallback if getcwd fails
            }
        fflush(stdout); 
    }


        //read Input
        if (fgets(input, MAX_BUFFER, in) == NULL) {
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



    }
    
    return 0;
}