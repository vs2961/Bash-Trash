#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

static void sighandler(int signo)
{
    if (signo == SIGINT)
    {
        printf("\n");
    }
}
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
    //if(arg_count == 2) printf("a0: %s, a1: %s, a2: %s\n", args[0], args[1], args[2]);
    //if(arg_count == 1) printf("a0: %s, a1: %s\n", args[0], args[1]);
    return args;
}

void redirect(char **args)
{
    int i = 0;
    while (args[i] != NULL)
    { // ls > hello.txt
        if (strcmp(args[i], ">") == 0)
        { //Change out
            int file_desc = open(args[i + 1], O_TRUNC | O_RDWR | O_CREAT, 0644);
            dup2(file_desc, 1);
            args[i] = NULL;
            return;
        }

        if (strcmp(args[i], ">>") == 0)
        { //Change out but append instead of overwriting
            int file_desc = open(args[i + 1], O_RDWR | O_CREAT | O_APPEND, 0644);
            dup2(file_desc, 1);
            args[i] = NULL;
            return;
        }

        if (strcmp(args[i], "<") == 0)
        { //Change stdin
            int file_desc = open(args[i + 1], O_RDONLY, 0644);
            dup2(file_desc, 0);
            args[i] = NULL;
            return;
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
    return new_buffer;
}

int main()
{
    char buffer[256];
    const int max_len = 256;
    while (1)
    {
        //Print bash_trash:[current directory]
        char curr_dir[256];
        getcwd(curr_dir, 256);
        printf("\nbash_trash:%s$ ", curr_dir);

        //Read stdin for user arguments and get rid of spaces.
        fgets(buffer, max_len, stdin);

        if (buffer[0] != NULL)
        {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n')
            {
                buffer[--len] = '\0';
            }
        }

        char *new_buffer = fix_input(buffer, max_len); //Must be freed, but do not free until end of program!
        printf("No Space: %s\n\n", new_buffer);

        //Count commands

        char **args = parse_args(new_buffer, ";"); //**args depends on no_space_buffer to work
        //Execute commands
        int i = 0;
        int backup_sdin = dup(STDIN_FILENO);
        for (i = 0; args[i] != NULL; i++)
        {
            char **pipe_args = parse_args(args[i], "|");
            int j = 0;
            // for (j = 0; pipe_args[j] != NULL; j++)
            // {
            //     printf("at i=%d — pipe_args[%d]: %s\n", i, j, pipe_args[j]);
            // }
            FILE *inpt = NULL;
            FILE *oupt = NULL;

            for (j = 0; pipe_args[j] != NULL; j++)
            {
                if (pipe_args[j + 1] != NULL)
                {

                    inpt = popen(pipe_args[j], "r");
                    int fd = fileno(inpt);

                    dup2(fd, STDIN_FILENO);
                }
                else
                {
                    run_child(pipe_args[j]);
                }
            }
            dup2(backup_sdin, STDIN_FILENO);
        }

        free(args);
        free(new_buffer);
    }
}
