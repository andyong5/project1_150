#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "parsing.h"

#define CMDLINE_MAX 512

struct Pipe
{
	char *cmd;
	char *args[3];
	int error;
} Pipe;

char *deblank(char *input) //https://stackoverflow.com/questions/13084236/function-to-remove-spaces-from-string-char-array-in-c
{
	int i, j;
	char *output = input;
	for (i = 0, j = 0; i < (int)strlen(input); i++, j++)
	{
		if (input[i] != ' ')
			output[j] = input[i];
		else
			j--;
	}
	output[j] = 0;
	return output;
}

void cd(char *full_cmd, struct command *cmd_struct)
{
	char *cmd_remove = deblank(cmd_struct->args);
	if (strcmp(cmd_remove, "..") == 0)
	{
		chdir("..");
		fprintf(stdout, "+ completed '%s' [%d]\n", full_cmd, 0);
	}
	else
	{
		if (chdir(cmd_remove) == 0)
		{
			fprintf(stdout, "+ completed '%s' [%d]\n", full_cmd, 0);
		}
		else
		{
			fprintf(stdout, "Error: no such directory\n");
			fprintf(stdout, "+ completed '%s' [%d]\n", full_cmd, 1);
		}
	}
}

void redirect(char *full_cmd, struct command *cmd_struct)
{
	printf("%s\n", cmd_struct->file);
	char *test2[] = {cmd_struct->cmd, "hello world", NULL};
	pid_t pid;
	pid = fork();
	int fd;
	if (pid == 0)
	{
		fd = open("test", O_RDWR | O_CREAT | O_TRUNC, 0644);
		dup2(fd, STDOUT_FILENO);
		close(fd);
		execvp(cmd_struct->cmd, test2);
		// perror("execvp");
		exit(1);
	}
	else if (pid > 0)
	{
		int status;
		waitpid(pid, &status, 0);
		fprintf(stdout, "+ completed '%s' [%d]\n", full_cmd, status);
	}
}

void pipe_cmd()
{
	struct Pipe pipes[3];
	pipes[0].cmd = "cat";
	pipes[0].args[0] = pipes[0].cmd;
	pipes[0].args[1] = "food";
	pipes[0].args[2] = NULL;

	pipes[1].cmd = "head";
	pipes[1].args[0] = pipes[1].cmd;
	pipes[1].args[1] = "-n 2";
	pipes[1].args[2] = NULL;

	pipes[2].cmd = "grep";
	pipes[2].args[0] = pipes[2].cmd;
	pipes[2].args[1] = "apple";
	pipes[2].args[2] = NULL;

	int pipe_size = (int)(sizeof(pipes) / sizeof(pipes[0]));
	int fd[2];
	int fd2[2];
	printf("%d pipe size\n", pipe_size);
	pipe(fd);
	for (int i = 0; i < (int)pipe_size; i++) {
		if(i %2 == 1 && i + 1 < (int)pipe_size) {
			int old = fd[0];
			printf("old = %d\n", old);
			pipe(fd);
			printf("fd[0] = %d after\n", fd[0]);
			printf("fd[1] = %d after\n", fd[1]);
			fd[0] = old;
		}
		pid_t pid;
		pid = fork();
		if (pid == 0)
		{
			if (i % 2 == 0)
			{
				printf("fd[0] = %d\n", fd[0]);
				printf("fd[1] = %d\n", fd[1]);
				close(fd[0]);
				dup2(fd[1], STDOUT_FILENO);
				close(fd[1]);
				execvp(pipes[i].cmd, pipes[i].args);
			}
			else
			{
				printf("fd[0] = %d\n", fd[0]);
				printf("fd[1] = %d\n", fd[1]);
				close(fd[1]);
				dup2(fd[0], STDIN_FILENO);
				close(fd[0]);
				execvp(pipes[i].cmd, pipes[i].args);
			}
		}
		else if (pid > 0)
		{
			int status;
			waitpid(pid, &status, 0);
			printf("fd2[0] = %d\n", fd[0]);
			printf("fd2[1] = %d\n", fd[1]);
			// fprintf(stdout, "+ completed [%d]\n", status);
		}
	}
}

int main(void)
{
	char cmd[CMDLINE_MAX];

	while (1)
	{
		char *nl;

		/* Print prompt */
		printf("sshell$ ");
		fflush(stdout);

		/* Get command line */
		fgets(cmd, CMDLINE_MAX, stdin);

		/* Print command line if stdin is not provided by terminal */
		if (!isatty(STDIN_FILENO))
		{
			printf("%s", cmd);
			fflush(stdout);
		}

		/* Remove trailing newline from command line */
		nl = strchr(cmd, '\n');
		if (nl)
			*nl = '\0';

		/* Builtin command */
		if (!strcmp(cmd, "exit"))
		{
			fprintf(stderr, "Bye...\n");
			break;
		}
		/* Regular command */

		struct cLine cline_struct = parse(cmd);
		struct command *cmd_struct = parse_cmd(cline_struct, 0);
		char *test[] = {cmd_struct->cmd, NULL};
		// char *test2[] = {cmd_struct->cmd, "test",NULL};
		// char *cmd_remove = deblank(cmd_struct->args);
		// printf("%s\n", cmd_struct->args);
		if (strcmp(cmd_struct->cmd, "cd") == 0)
		{
			cd(cmd, cmd_struct);
		}
		else if (cmd_struct->isRedirected)
		{
			redirect(cmd, cmd_struct);
		}
		else if (cmd_struct->isPiped)
		{
			pid_t pid;
			pid = fork();
			if (pid == 0)
			{
				pipe_cmd();
			}
			else if (pid > 0)
			{
				int status;
				waitpid(pid, &status, 0);
				fprintf(stdout, "+ completed '%s' [%d]\n", cmd, status);
			}
		}
		else
		{
			pid_t pid;
			pid = fork();
			if (pid == 0)
			{
				if (strcmp(cmd_struct->cmd, "pwd") == 0)
				{
					execvp(cmd_struct->cmd, test);
				}
				else
				{
					execvp(cmd_struct->cmd, &cmd_struct->args);
					// perror("execvp");
					exit(1);
				}
			}
			else if (pid > 0)
			{
				int status;
				waitpid(pid, &status, 0);
				fprintf(stdout, "+ completed '%s' [%d]\n", cmd, status);
			}
		}
	}

	return EXIT_SUCCESS;
}
