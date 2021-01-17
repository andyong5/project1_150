project1: project1.c
	gcc -Wall -Wextra -o project1 project1.c parsing.c
clean:
	rm -rf *.o project1