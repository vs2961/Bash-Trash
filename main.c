#include <stdio.h>
#include "parsing.h"
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
static void sighandler(int signo)
{
    if (signo == SIGINT)
    {
        printf("\n");
    }
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
        if (fgets(buffer, max_len, stdin) != NULL)
            ;
        {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n')
            {
                buffer[--len] = '\0';
            }
        }

        char *new_buffer = fix_input(buffer, max_len); //Must be freed, but do not free until end of program!
        printf(" %s\n\n", new_buffer);

        //Count commands

        char **args = parse_args(new_buffer, ";"); //**args depends on no_space_buffer to work
        //Execute commands
        int i = 0;
        int backup_sdin = dup(STDIN_FILENO);
        for (i = 0; args[i] != NULL; i++)
        {
            char **pipe_args = parse_args(args[i], "|");
            int j = 0;
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
