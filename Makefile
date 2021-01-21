sshell: sshell.c
	gcc -Wall -Wextra -Werror -o sshell sshell.c parsing.c
clean:
	rm -rf *.o sshell
