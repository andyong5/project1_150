#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMDLINE_MAX 512

struct command{
	char *cmd;
	char *arg;
	char *args[3];
} command;

struct command parse_cmd(char *full_cmd){
	char *token;
	token = strtok(full_cmd, " ");
	char *cmd = token;
	token = strtok(NULL, "");

	// while(token != NULL){
	// 	printf("%s\n", token);
	// 	token = strtok(NULL, " ");
	// }
	
	struct command c1;
	c1.cmd = cmd;
	c1.arg = token;
	c1.args[0] = cmd;
	c1.args[1] = token;
	c1.args[2] = NULL;

	return (c1);
}

void get_args(char *cmd){
	char *token;
	token = strtok(cmd, " ");
	
	while(token != NULL){
		printf(" %s\n", token);
		token = strtok(NULL," ");
	}

}

void cd(char *full_cmd, struct command cmd){
	if(strcmp(cmd.arg, "..") == 0){
		chdir("..");
		fprintf(stdout, "+ completed '%s' [%d]\n", full_cmd, 0);
	}
	else{
		if(chdir(cmd.arg) == 0){
			fprintf(stdout, "+ completed '%s' [%d]\n", full_cmd, 0);
		}
		else{
			fprintf(stdout, "Error: no such directory\n");
			fprintf(stdout, "+ completed '%s' [%d]\n", full_cmd, 1);
		}
	}
}


int main(void)
{
	char cmd[CMDLINE_MAX];

	while (1) {
		char *nl;

		/* Print prompt */
		printf("sshell$ ");
		fflush(stdout);

		/* Get command line */
		fgets(cmd, CMDLINE_MAX, stdin);

		/* Print command line if stdin is not provided by terminal */
		if (!isatty(STDIN_FILENO)) {
			printf("%s", cmd);
			fflush(stdout);
		}

		/* Remove trailing newline from command line */
		nl = strchr(cmd, '\n');
		if (nl)
			*nl = '\0';

		/* Builtin command */
		if (!strcmp(cmd, "exit")) {
			fprintf(stderr, "Bye...\n");
			break;
		}
		/* Regular command */
		//get_args(cmd);

		char full_cmd[CMDLINE_MAX];
		strcpy(full_cmd, cmd);
		struct command cmd_struct = parse_cmd(cmd);
		
		if(strcmp(cmd_struct.cmd, "cd") == 0){
			cd(cmd, cmd_struct);
		}else{
			pid_t pid; 
			pid = fork();
			if(pid == 0){
				if(strcmp(cmd_struct.cmd, "pwd") == 0 && strcmp(cmd_struct.arg, "") != 0 ){
					char *pwd_cmd[] = {cmd_struct.cmd, NULL};
					execvp(cmd_struct.cmd, pwd_cmd);
				}
				else{
					execvp(cmd_struct.cmd, cmd_struct.args);
					// perror("execvp");
					exit(1);
				}
			} else if (pid > 0 ){
				int status;
				waitpid(pid, &status, 0);
				fprintf(stdout, "+ completed '%s' [%d]\n", full_cmd, status);
			}
		}
		
	}

	return EXIT_SUCCESS;
}

