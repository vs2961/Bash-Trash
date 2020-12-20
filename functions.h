static void sighandler(int signo);
char *strip_cmd(char *command);
char **parse_args(char *line, char *delim);
void redirect(char **args);
void run_child(char *command, int in, int out);
void run_child_with_pipe(char * command);
char *get_rid_of_spaces(char *buffer, int max_size);