char *strip_cmd(char *command);
char **parse_args(char *line, char *delim);
void redirect(char **args);
void run_child(char *command);
char * fix_input(char *buffer, int max_size);
