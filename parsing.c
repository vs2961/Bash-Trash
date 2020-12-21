#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

char *strip_cmd(char *command)
{
    // printf("command: %s\n", command);
    if (!*command)
    {
        return command;
    }
    int i;
    int last_nw;
    int first_nw = -1;
    for (i = 0; *(command + i); i++)
    {

        if (*(command + i) != '\n' && *(command + i) != ' ' && first_nw == -1)
        {
            first_nw = i;
        }
        if (*(command + i) != ' ' && *(command + i) != '\n')
        {
            last_nw = i;
        }
    }
    *(command + last_nw + 1) = '\0';
    // printf("last_nw: %d\n", last_nw);
    return command + first_nw;
}

char **parse_args(char *line, char *delim)
{
    //Using *line as the backing bytes, this function creates an array of tokens
    //*line can not change, or else this will break
    //*line must have at least one argument

    // preparing the size of the args array
    int arg_count = 1;
    {
        int i = 0;
        int length_line = strlen(line);
        for (i = 0; i < length_line; i++)
        {
            if (line[i] == *delim)
                arg_count++;
        }
    }

    char *token;
    char *p = line;
    int i = 0;
    char **args = malloc((arg_count + 1) * sizeof(char *)); //Plus one for the NULL terminator
    args[arg_count] = NULL;

    while (p)
    {
        token = strsep(&p, delim);
        args[i] = token;
        i++;
    }
    return args;
}

void redirect(char **args)
{
    int i = 0;
    while (args[i] != NULL)
    {
        if (strcmp(args[i], ">") == 0)
        { //Change out
            int file_desc1 = open(args[i + 1], O_TRUNC | O_RDWR | O_CREAT, 0644);
            dup2(file_desc1, STDOUT_FILENO);
            args[i] = NULL;
        }

        else if (strcmp(args[i], ">>") == 0)
        { //Change out but append instead of overwriting
            int file_desc2 = open(args[i + 1], O_RDWR | O_CREAT | O_APPEND, 0644);
            dup2(file_desc2, STDOUT_FILENO);
            args[i] = NULL;
        }

        else if (strcmp(args[i], "<") == 0)
        { //Change stdin
            int file_desc3 = open(args[i + 1], O_RDONLY, 0644);
            dup2(file_desc3, STDIN_FILENO);
            args[i] = NULL;
        }
        i++;
    }
}

void run_child(char *command)
{
    char **args = parse_args(strip_cmd(command), " ");

    if (!strcmp(args[0], "exit"))
    {
        exit(0);
    }
    else if (!strcmp(args[0], "cd"))
    {
        chdir(args[1]);
    }
    else
    {
        int f = fork();
        if (f)
        { //The parent
            int status;
            wait(&status);
        }
        else
        { //The child
            /*if(strcmp(args[0], "") == 0) {
                printf("WARNING: One of your arguments was blank!\n");
                free(args);
                return;
            } */
            int backup_sdout = dup(STDOUT_FILENO);
            int backup_sdin = dup(STDIN_FILENO);

            redirect(args);
            int check_error = execvp(args[0], args);
            dup2(backup_sdout, STDOUT_FILENO);
            dup2(backup_sdin, STDIN_FILENO);
            if (check_error == -1)
            {
                printf("Error with argument %s: %s\n", args[0], strerror(errno));
            }
        }
        free(args);
    }
}

char *fix_input(char *buffer, int max_size)
{
    char *new_buffer = malloc(max_size * sizeof(char));
    int i = 0;
    while (*buffer == ' ' || *buffer == '\t' || *buffer == '\n')
    {
        i++;
    }
    int pos = -1;
    while (i < max_size && buffer[i])
    {
        if (buffer[i] == '\\')
        {
            new_buffer[++pos] = buffer[i + 1];
            i++;
        }
        else if (buffer[i] == ' ' && new_buffer[pos] != ' ')
        {
            new_buffer[++pos] = buffer[i];
        }
        else if (buffer[i] == '>' && buffer[i + 1] == '>')
        {
            if (new_buffer[pos] != ' ')
            {
                new_buffer[++pos] = ' ';
            }
            new_buffer[++pos] = buffer[i];
            new_buffer[++pos] = buffer[i + 1];
            new_buffer[++pos] = ' ';
            i++;
        }
        else if (buffer[i] == '>' || buffer[i] == '<' || buffer[i] == '|')
        {
            if (new_buffer[pos] != ' ')
            {
                new_buffer[++pos] = ' ';
            }
            new_buffer[++pos] = buffer[i];
            new_buffer[++pos] = ' ';
        }
        else if (buffer[i] != ' ' && buffer[i] != '\t')
        {
            new_buffer[++pos] = buffer[i];
        }
        i++;
    }
    new_buffer[++pos] = '\0';
    return new_buffer;
}