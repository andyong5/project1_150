## Summary
This program, ‘sshell’, is a shell. 
‘’’
$ ./shell
‘’’
## Implementation
The implementation of this program follows four distinct steps:
Parse input
Check for errors
Execute that specific command (cd, redirect, pipe, set) 
All other commands use execvp

## Parse Input
For the parsing section of the assignment two functions were made. The first
function is the the parse() function that parses the input into an array of
strings so that each element in the array is either a word, >, or |. Originally
the strok function was used, but it was not able to separate > or | if there was
no space between them. The function parse() returns a struct cLine that consists
of an array of strings and the number of elements in the array. The cLine
struct was made because c lacks a function that returns the number of elements
in an array. The second function parse_cmd() uses the output of parse to create
a struct called command. The struct keeps track of all the important details of
the user inputs. It holds the name of the system call, its arguments, where it
should print the output if it's being redirected, and if it is part of a pipe it
will point to the struct that its output is being piped to. The struct acts like
a linked list if it is being piped because it allows us to recursively call the
parse_cmd function to create a new command for the things after a |.

## Check for errors.
Most parsing errors were handled by the parse_cmd function. If the parse_cmd
function notices an error it will return a command struct with a true error
value, and with cmd being the error message that would be printed. Errors that
are not caught by parse_cmd() are caught before their execution so that the
sshell does not crash.
 
## Execute specific commands (cd, redirect, pipe, set) 

##cd
For cd, we just used the chdir() function and printed out errors if there were
any. We had to check for if it was NULL, .., and if it was possible to cd into
that specific directory.

##redirect
To do redirect, we needed to fork a child since we need to use execvp to execute
a command. We made sure to check if there were no output files and if the file
couldn’t be opened. We also had to redirect the output of the terminal to the
text file using dup2. 


## Pipe
To do the pipe command, I used this post
https://stackoverflow.com/questions/6542491/how-to-create-two-processes-from-a-single-parent
to explain how to create two children from a single parent. Which
is by forking and forking again in the parent of the first fork. So now the
parent has two children. The rest of the pipe function was my own thought
process. We accomplished this by always having two pipes, one for the old file
descriptors and the new file descriptors. To execute the pipe, we did it
iteratively by looping every 2 commands. We had to do it this way because each
command had to run simultaneously to make sure there was no overflow of the
pipe. Each new 2 commands, I would replace the old file descriptors with the new
file descriptors and call pipe again on the new file descriptors. This way I
didn’t need to create an array to store all the file descriptors. I had two
cases for if the number of commands was even or odd. If it was odd, we only
needed to fork once. If it was even, it ran regularly with 2 children and 1
parent. The way our function works is by two cases. One case where the input
comes from the pipe and the redirect of the output needs to go to a new pipe.
The other case is when it only reads from the pipe and the output goes to the
standard stdout for the terminal. In between these two cases, we would get the
input from the old pipe and redirect the output of the command to the new pipe.
To accomplish this, we needed to get the data from the old pipe, then redirect
the output to the new pipe. We repeated this until it was the last command. To
keep track of the statuses for each command, we created an array that was the
same length as the number of the commands. In the parent, we would wait and get
the value of the return value of the function and push that into the array. Once
the function was the last one, we would pass the array to a function to iterate
through each command and print out their respective errors.   

## Set
For the set command we made an array of size 26 filled with empty strings for
all characters a-z that can be set. When the set command is used it checks if 
the variable has a proper name. If it is a proper variable then the array we 
made earlier will go to the position for the letter a-z to make it equal to what
the user wanted. If a variable is being used signified by $ the parse_cmd 
function  will catch it and make sure it is replaced by what is stored in the
array.

## All other commands
For all other commands, we would need to fork a child and execute the command
and wait in the parent for the child to finish executing. We were able to get 
the value of this by using WEXITSTATUS. 
