#include <stdio.h>
#include "functions.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char **argv)
{
    char buffer[256];
    const int max_len = 256;
    char curr_dir[256];
    int printBashTrash = 1; //Do we print bash_trash:[current directory]?
    while (1)
    {   
        getcwd(curr_dir, 256);
        //Print bash_trash:[current directory]
        if(argc == 2 && strcmp(argv[1], "hide") == 0);
        else printf("\nbash_trash:%s$ ", curr_dir);
        if(fgets(buffer, max_len, stdin) == NULL){
            break;
        } 

        if(buffer[0] == '\n') continue;
        if (buffer != NULL)
        {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n')
            {
                buffer[--len] = '\0';
            }
        }

        char *no_space_buffer = get_rid_of_spaces(buffer, max_len); //Must be freed, but do not free until end of program!

        char **args = parse_args(no_space_buffer, ";"); //**args depends on no_space_buffer to work
        
        //Execute commands
        int i = 0;
        for (i = 0; args[i] != NULL; i++)
        {
            run_child_with_pipe(args[i]);
        }

        free(args);
        free(no_space_buffer);
    }
}