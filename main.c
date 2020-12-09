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

char *strip_cmd(char *command) {
    if (!*command) {
        return command;
    }
    int i;
    int last_nw;
    int first_nw = -1;
    for (i = 0; *(command + i); i++) {
        if (*(command + i) != '\n' && *(command + i) != ' ' && first_nw == -1) {
            first_nw = i;
        }
        if (*(command + i) != ' ' && *(command + i) != '\n') {
            last_nw = i;
        }
    }
    *(command + last_nw + 1) = '\0';
    return command + first_nw;
}

char **parse_args(char *line, char *delim) {
    char *token;
    char *p = line;
    int i = 0;
    char **args = malloc(5 * sizeof(char *));
    while (p) {
        p = strip_cmd(p);
        token = strsep(&p, delim);
        args[i] = malloc(5 * sizeof(char));
        args[i] = token;
        i++;
    }
    return args;
}

void run_child(char *command) {
    char **args = parse_args(command, " ");
    if (!strcmp(args[0], "exit")) {
        exit(0);
    }
    else if (!strcmp(args[0], "cd")) {
        chdir(args[1]);
    }
    else {
        int f = fork();
        if (f)
        {
            int status;
            wait(&status);
        }
        else
        {
            execvp(args[0], args);
        }
    }
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
        char **args = parse_args(buffer, ";");
        int i;
        for (i = 0; args[i] != NULL; i++) {
            run_child(strip_cmd(args[i]));
        }
        free(args);
    }
}
