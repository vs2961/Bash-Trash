# Project 01: Shell
by `Michael Nath`, `Victor Siu`, and `Haotian Gan` 
> TNPG: Bash_Trash
## Features
* can execute commands like ls, cd, less, and more!
* parses multiple commands separated by `;` on one line
* uses `fork()` *(oh snap!)* to create child processes
* presents an authentic bash prompt
* offers support for redirection (`<` and `>` and `>>`) 
* multi-piping (`|`) supported
* extraneous spaces supported (commands can have more than one space between arguments)
* <b> NOTE: Use "./main hide" to prevent our shell from printing "bash_trash:[current-directory]" before each command.
        For example: use "./main hide < test_cmds.txt" to feed in the commands you wrote in test_cmds.txt while only seeing output. </b>
---
## Attempted
* Using "" to encapsulate arguments (not supported)


## Bugs
* Haotian faced an issue on his WSL2 virtual ubuntu installation where ./main < test_cmds.txt ran in an infinite loop with weird undefined inputs after the test_cmds.txt EOF was     reached. Victor and Michael, on their mac systems, did not have the same issue when the exact same code was run. Haotian found that copying test_cmds.txt's content to another     file, and running ./main with that new file as input, fixed the issue for some reason. 
