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
    //Stop the program
    if (signo == SIGINT)
    {
        printf("\n");
    }
}

char *strip_cmd(char *command)
{   
    //Input: A string with a null terminator
    //Output: The same string, but it begins at the first non whitespace character
    //        and it ends at a non whitespace character (NULL terminator is put in)
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
    //Input: a line to parse with a specified delimiter
    //Using *line as the backing bytes, this function creates an array of tokens
    //*line can not change, or else this will break
    //*line must have at least one argument (Cannot be blank)

    int arg_count = 1;
    {
        int i = 0;
        for(i = 0; i < strlen(line); i++){
            if(line[i] == *delim) arg_count++; 
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

void redirect(char **args){
    //Input: A child's arguments
    //Output The child is redirected if its arguments contain >>, > or < 
    int i = 0;
    while(args[i] != NULL){ // ls > hello.txt
        if(strcmp(args[i],">") == 0) { //Change out
            //printf("Changed stOUT.\n");
            int file_desc = open(args[i+1], O_TRUNC | O_WRONLY | O_CREAT, 0644); 
            dup2(file_desc, 1);
            args[i] = NULL;
        } else

        if(strcmp(args[i],">>") == 0) { //Change out but append instead of overwriting
            //printf("Appending stdOUUT.\n");
            int file_desc = open(args[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644); 
            dup2(file_desc, 1);
            args[i] = NULL;
            
        } else
        
        if(strcmp(args[i],"<") == 0) { //Change stdin
            //printf("Changed stdIN.\n");
            int file_desc = open(args[i+1], O_RDONLY, 0644); 
            dup2(file_desc, 0);
            args[i] = NULL;
            
        }
        i++;
    }
}


void run_child(char *command, int in, int out)
{
    //Input: A single command for the child to run, along with an 
    //       stdin to redirect to, and stdout to redirect to 
    //       Use in=0, and out=1 for no redirection
    //       <, >, >> symbols take precedence over in and out
    
    //Output: The command is run, and input output are redirected appropriately
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
            //printf("Parent finished\n");
        }
        else
        { 
            //The child
            if(strcmp(args[0], "") == 0) {
                printf("WARNING: One of your arguments was blank!\n");
                free(args);
                return;
            } 
            
            //Read from another fd (piping purposes)
            if (in != 0){
                dup2 (in, 0);
            }

            if (out != 1){
                dup2 (out, 1);
            }

            //If the command contains redirectional stuff, have that take precendence. 
            redirect(args);
            int check_error = execvp(args[0], args);
            if (check_error == -1) {
                printf("Error with argument %s: %s\n", args[0],strerror(errno));
            }
        }
        free(args);
    }
}

void run_child_with_pipe(char * command){
    //Note: run_child does not manage pipes, whereas this function does.
    //      So use this for passing commands into for running. 
    //Input: A single command that either doesn't have or has pipes
    //Output: Checks for pipes, and if there are pipes, takes care of that.

    char **commandWithPipes = parse_args(command, "|");
    int k = 0;
    while(commandWithPipes[k] != NULL){
        commandWithPipes[k] = strip_cmd(commandWithPipes[k]);
        //printf("commandWithPipes: %s\n", commandWithPipes[k]);
        k++;
    }


    //If the first argument was a NULL for some reason
    if(commandWithPipes[0] == NULL){
        printf("run_child_with_pipe's first argument was NULL?!\n");
        return;
    }

    //If there were no pipes and the parsed input looked liked this:
    //(command) (NULL)
    if(commandWithPipes[1] == NULL) {
        run_child(commandWithPipes[0], 0, 1);
        return;
    } 

    int i = 0;
    
    //pipe(fd);

    char input_template[] = "/tmp/fileXXXXXX";
    char output_template[] = "/tmp/fileXXXXXX";

    int input, output;
    input = mkstemp(input_template);
    output = mkstemp(output_template);
    unlink(input_template);
    unlink(output_template);

    // fd[0] = open("input.txt", O_TRUNC | O_RDWR | O_CREAT, 0644);
    // fd[1] = open("output.txt", O_TRUNC | O_RDWR | O_CREAT, 0644);
    for(i = 0; commandWithPipes[i+1] != NULL; i++){
        lseek(input, 0, SEEK_SET);
        run_child(commandWithPipes[i], input, output);

        close(input);
        input = output;
        char output_template[] = "/tmp/fileXXXXXX";
        output = mkstemp(output_template);
        unlink(output_template);
    }
    lseek(input, 0, SEEK_SET);
    run_child(commandWithPipes[i], input, 1);
}



char *get_rid_of_spaces(char *buffer, int max_size){
    //Input: A buffer from fgets() along with the max size of the buffer
    //Output: a new buffer, with extra spaces between words. 
    char *newBuffer = malloc(max_size * sizeof(char));          
    int i = 0;
    int pos = 0;
    bool first_met = false;
    bool second_met = false;
    for(i = 0; i < max_size; i++){
        //Im not a space
        if(buffer[i] == ';' || buffer[i] == '|') first_met = second_met = false;
        if(buffer[i] != ' '){
            if(first_met && second_met) {
                newBuffer[pos++] = ' ';
                first_met = second_met = false;
            }
            newBuffer[pos++] = buffer[i];
            continue;
        }
        //Im a space
        if( (i+1) < max_size && buffer[i+1] != ' ' && buffer[i+1] != ';' && buffer[i+1] != '\0') second_met = true;
        if( (i-1) >= 0 && buffer[i-1] != ' ' && buffer[i-1] != ';' && buffer[i-1] != '\0') first_met = true;
        
    }
    return newBuffer;
}

