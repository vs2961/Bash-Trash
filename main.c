#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>


static void sighandler(int signo) {
    if (signo == SIGINT) {
        printf("\n");
    }
}

char **parse_args(char *line) {
    char **args = malloc(5 * sizeof(char*));
    char p[256];
    strcpy(p, line);
    char *q = p;
    char *token;
    for (int i = 0; i < 5 && q != NULL; i++) {
        token = strsep(&q, " ");
        args[i] = malloc(5 * sizeof(char));
        args[i] = token;
    }
    return args;
}

int main() {
    char buffer[256];
    while (1) {
        char curr_dir[256];
        getcwd(curr_dir, 256);
        printf("\nbash_trash:%s$ ", curr_dir);
        if (fgets(buffer, 256, stdin) != NULL) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[--len] = '\0';
            }
        }
        char **args = parse_args(buffer);
        if (!strcmp(args[0], "exit")) {
            exit(0);
        } else if (!strcmp(args[0], "cd")) {
            chdir(args[1]);
        } else {
            int f = fork();
            if (f) {
                int status;
                wait(&status);
            } else {
                execvp(args[0], args);
            }
            free(args);
        }
    }
}
