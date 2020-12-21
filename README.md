# Bash Trash
by `Michael Nath`, `Victor Siu`, and `Haotian Gan` 
> TNPG: Bash_Trash
## Features
* <b> Each command can only be at most 255 characters long. </b>
* Forks and executes commands
* Parses multiple commands separated by `;` on one line
* Presents an genuine bash prompt
* Offers support for redirection (`<` and `>` and `>>`)
* Multi-piping (`|`) supported
* Extraneous spaces supported (commands can have more than one space between arguments)

## Attempted
* Using "" to encapsulate arguments

## Bugs
* exit sometimes doesn't run (if it doesn't exit, use ctrl-C)
* messing with the fix_input() command by typing ">>>>>>>" causes it to read 
  it in as multiple ">>" and a ">" at the end depending on how many characters there are.
* typing pipe without arguments causes a segfault (tried to fix this but segfaults are syntax errors, right?)

**parsing.c**

```
=-=-=-=-= static void sighandler() =-=-=-=-=
Input: int signo

* Catches SIGINT(ctrl-c) and will exit any child process running
* The bash prompt (parent) will still exit when SIGINT is signaled

========================

=-=-=-=-= char * strip_cmd() =-=-=-=-=
Input: char * command

Returns: Pointer to beginning of stripped char *

* A basic strip function. Removes whitespace from both sides (" ", "\t", "\n")

=========================

=-=-=-=-= char * fix_input() =-=-=-=-=
Inputs: char * buffer, int MAX_SIZE

Returns: Pointer to beginning of fixed command input

* Removes extra spaces between characters
* Adds spaces before and after "<", ">>", ">", and "|"
* \ is an escape character

=========================

=-=-=-=-= char ** parse_args() =-=-=-=-=
Inputs: char * line, char * delim

Returns: Array of strings separated by delim

* Uses malloc() to create and the array (must free it later)

=========================

=-=-=-=-= void run_child() =-=-=-=-=
Input: char * command

* Runs the command inputted by the user
* Only takes in a well-formed command with spaces between all arguments

=========================

=-=-=-=-= void redirect() =-=-=-=-=
Input: char ** args

* Loops through tokens created by parse_args()
* Changes stdin and stdout to the file based on the parameter after >, >>, or <.
* Putting a sequence of > will cause each file to be opened, but only the last
file will be written to.
==========================

```

**main.c**

```
\*/=\*/=\*/ main() \*/=\*/=\*/

* Reads input from the command line, separated by \n
* Calls fix_input() to make the input prettier
* Parses the commands by ";", then by "|".
* Gets output from commands using popen()
* Runs the last function using run_child()
* Frees any memory that was malloc-ed by parse_args

```
