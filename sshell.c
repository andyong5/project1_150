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

void cd(char *full_cmd, struct command *cmd_struct) {

	if(cmd_struct->args[1] == NULL){
		fprintf(stderr, "Error: cannot cd into directory\n");
		fprintf(stderr, "+ completed '%s' [%d]\n", full_cmd, 1);
		return;
	} 
	if (strcmp(cmd_struct->args[1], "..") == 0) {
		chdir("..");
		fprintf(stderr, "+ completed '%s' [%d]\n", full_cmd, 0);
	} 
	else {
		if (chdir(cmd_struct->args[1]) == 0) {
			fprintf(stderr, "+ completed '%s' [%d]\n", full_cmd, 0);
		} 
		else {
			fprintf(stderr, "Error: cannot cd into directory\n");
			fprintf(stderr, "+ completed '%s' [%d]\n", full_cmd, 1);
		}
	}
}

void redirect(char *full_cmd, struct command *cmd_struct) {
	
	if(strcmp(cmd_struct->file,"") == 0){
		fprintf(stderr, "Error: no output file\n");
	}
	pid_t pid;
	pid = fork();
	int fd;
	if (pid == 0) {
		fd = open(cmd_struct->file, O_RDWR | O_CREAT | O_TRUNC, 0644);
		if(fd == -1){
			fprintf(stderr, "Error: cannot open output file\n");
			exit(1);
		} 
		else {
			dup2(fd, STDOUT_FILENO);
			close(fd);

			execvp(cmd_struct->cmd, cmd_struct->args);
		}
	} 
	else if (pid > 0) {
		int status;
		waitpid(pid, &status, 0);
		int error = WEXITSTATUS(status);
		if(error != 1)
			fprintf(stderr, "+ completed '%s' [%d]\n", full_cmd, error);
	}
}

void print_error(int status[], int size, char* full_cmd){

	fprintf(stderr, "+ completed '%s' ", full_cmd);
	for(int i = 0; i < size; i++){
		fprintf(stderr, "[%d]", status[i]);
	}
	fprintf(stderr,"\n");
}
	
void pipe_cmd(char* full_cmd, struct command* cmd_struct) {
	int pipe_size = countPipes(cmd_struct) + 1;
	int statuses[pipe_size];
	int fd[2];
	int old[2];
	bool last_cmd = false;
	
	printf("%d\n", pipe_size);
	pid_t pi_d;
	pid_t pid;
	struct command* cur_cmd = cmd_struct;
	pipe(old);
	bool re = false;
	for(int i = 0; i < pipe_size; i+=2) { //https://stackoverflow.com/questions/6542491/how-to-create-two-processes-from-a-single-parent
		if(i == pipe_size -1){ //for odd number of pipes
			if(cmd_struct->isRedirected)
				re = true;
			pid_t pi_d2 = fork();
			if(pi_d2 != 0 ){
				close(fd[0]);
				close(fd[1]);
				int status;
				waitpid(pi_d2, &status, 0);
				int error = WEXITSTATUS(status);
				statuses[i] = error;
				print_error(statuses, pipe_size, full_cmd);
				break;
			}else {
				close(fd[1]);
				dup2(fd[0], STDIN_FILENO);
				close(fd[0]);
				if(re) {
					redirect(cur_cmd->cmd, cur_cmd);
				} else{
					execvp(cur_cmd->cmd, cur_cmd->args);
				}
				
			}
		}
		if(i + 2 == pipe_size)
			last_cmd = true;
		if(i > 0){
			close(old[0]);
			close(old[1]);
			old[0] = fd[0];
			old[1] = fd[1];
		}
		pipe(fd);
		pi_d = fork();
		if(pi_d == 0){
			if(i == 0){
				close(old[0]);
				dup2(old[1], STDOUT_FILENO);
				close(old[1]);

				close(fd[0]);
				close(fd[1]);
				execvp(cur_cmd->cmd, cur_cmd->args);
			} else{
				close(old[1]);
				dup2(old[0], STDIN_FILENO);
				close(old[0]);

				close(fd[0]);
				dup2(fd[1], STDOUT_FILENO);
				close(fd[1]);
				execvp(cur_cmd->cmd, cur_cmd->args);
			}
		}
		cur_cmd = cur_cmd->next;
		if(pi_d > 0){
			pid = fork();
			if(pid > 0){
				close(old[0]);
				close(old[1]);
				int status1;
				int status2;
				waitpid(pi_d, &status1, 0);
				waitpid(pid, &status2, 0);
				int error1 = WEXITSTATUS(status1);
				int error2 = WEXITSTATUS(status2);
				statuses[i] = error1;
				statuses[i+1] = error2;
				if(last_cmd)
					print_error(statuses, pipe_size, full_cmd);
			} else if(pid == 0){
				if(last_cmd){
					close(old[1]);
					dup2(old[0], STDIN_FILENO);
					close(old[0]);

					close(fd[0]);
					close(fd[1]);
					execvp(cur_cmd->cmd, cur_cmd->args);
				} else {
					close(old[1]);
					dup2(old[0], STDIN_FILENO);
					close(old[0]);

					close(fd[0]);
					dup2(fd[1], STDOUT_FILENO);
					close(fd[1]);
					execvp(cur_cmd->cmd, cur_cmd->args);
				}
			}
		}
		cur_cmd = cur_cmd->next;
	}
}

int main(void) {
	
	char cmd[CMDLINE_MAX];
	char** sets = malloc(sizeof(char*)*26);
	for( unsigned int i = 0; i <= 26; i++) {
		sets[i] = "";
	}
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
			fprintf(stderr, "+ completed 'exit' [0]\n");
			break;
		}
		/* Regular command */
		if(strcmp("", cmd) == 0){
			continue;
		}

		struct command *cmd_struct = parse_cmd(parse(cmd), 0, sets);

		if(cmd_struct == NULL){
			continue;
		}
			
		if (cmd_struct->error) {
			fprintf(stderr, "%s\n", cmd_struct->cmd);
		} 
		else if (strcmp(cmd_struct->cmd, "cd") == 0) {
			cd(cmd, cmd_struct);
		} 
		else if (cmd_struct->isRedirected) {
			redirect(cmd, cmd_struct);
		} 
		else if (cmd_struct->isPiped){
			pipe_cmd(cmd, cmd_struct);
		} 
		else if (strcmp(cmd_struct->cmd, "set") == 0) {	
			if(cmd_struct->args[1] == NULL || strlen(cmd_struct->args[1]) != 1 || cmd_struct->args[1][0] < 'a' || cmd_struct->args[1][0] > 'z') {
				fprintf(stderr, "Error: invalid variable name\n");
				fprintf(stderr, "+ completed '%s' [%d]\n", cmd, 1);
			} else {
				int index = (int)(cmd_struct->args[1][0]) - 'a';
				if (strcmp(sets[index], "") == 0)
					sets[index] = malloc(sizeof(char)*strlen(cmd_struct->args[2]));
				else 
					sets[index] = realloc(sets[index],sizeof(char)*strlen(cmd_struct->args[2]));
				
				strcpy(sets[index], cmd_struct->args[2]);
				fprintf(stderr, "+ completed '%s' [%d]\n", cmd, 0);
			}

		} 
		else {
			pid_t pid;
			pid = fork();
			if (pid == 0) {
				if (strcmp(cmd_struct->cmd, "pwd") == 0) {
					char *arr[] = {cmd_struct->cmd, NULL};
					execvp(cmd_struct->cmd, arr);
				} 
				else {
					execvp(cmd_struct->cmd, cmd_struct->args);
					exit(1);
				}
			}
			else if (pid > 0) {
				int status;
				waitpid(pid, &status, 0);
				int error = WEXITSTATUS(status);
				if(error == 1)
					fprintf(stderr, "Error: command not found\n");
				fprintf(stderr, "+ completed '%s' [%d]\n", cmd, error);
			}
		}
	}
	return EXIT_SUCCESS;
}
