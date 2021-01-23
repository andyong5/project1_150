sshell: sshell.o parsing.o
	gcc -Wall -Wextra -Werror -o sshell sshell.o parsing.o

sshell.o: sshell.c parsing.h
	gcc -Wall -Wextra -Werror -c -o sshell.o sshell.c

parsing.o: parsing.c parsing.h
	gcc -Wall -Wextra -Werror -c -o parsing.o parsing.c

clean:
	rm -rf *.o sshell sshell.o parsing.o
