#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parsing.h"

struct cLine parse(char* str){
	char** retVal = malloc(sizeof(char*));
	int numEle = 0;
	bool wasEnd = true;
	int curStrLen = 0;
	
	for(unsigned int i = 0; i < strlen(str); i++) {
		if (wasEnd && (str[i] == '|' || str[i] == '>')){
			wasEnd = false;
			numEle++;
			retVal = realloc(retVal, sizeof(char*)*numEle);
			retVal[numEle - 1] = malloc(sizeof(char)*2);
			retVal[numEle - 1][0] = str[i];
			retVal[numEle - 1][1] = '\0';
			curStrLen++;
		} else if (!wasEnd && (str[i] == '|' || str[i] == '>')) {
			wasEnd = true;
			retVal[numEle - 1] = realloc(retVal[numEle - 1], (curStrLen + 1) * sizeof(char));
			retVal[numEle - 1][curStrLen] = '\0';
			curStrLen = 0;
			numEle++;
			retVal = realloc(retVal, sizeof(char*)*numEle);
			retVal[numEle - 1] = malloc(sizeof(char)*2);
			retVal[numEle - 1][0] = str[i];
			retVal[numEle - 1][1] = '\0';
		} else if(wasEnd && str[i] == ' '){
			continue;
		} else if(wasEnd && str[i] != ' '){
			wasEnd = false;
			numEle++;
			retVal = realloc(retVal, sizeof(char*)*numEle);
			retVal[numEle - 1] = malloc(sizeof(char));
			retVal[numEle - 1][0] = str[i];
			curStrLen++;
		} else if(!wasEnd && str[i] != ' ') {
			curStrLen++;
			retVal[numEle - 1] = realloc(retVal[numEle - 1], curStrLen*sizeof(char));
			retVal[numEle - 1][curStrLen - 1] = str[i];
		} else if(!wasEnd && str[i] == ' ') {
			wasEnd = true;
			retVal[numEle - 1] = realloc(retVal[numEle - 1], (curStrLen + 1) * sizeof(char));
			retVal[numEle - 1][curStrLen] = '\0';
			curStrLen = 0;
		}
	}
	struct cLine ret;
	ret.tokens = retVal;
	ret.size = numEle;
	return ret;
}

struct command* parse_cmd(struct cLine args, int start, char** sets){
	if(start == args.size)
		return NULL;
	if(strcmp(args.tokens[start], ">") == 0 || strcmp(args.tokens[start], "|") == 0) {
		struct command* error = malloc(sizeof(struct command));
		error->cmd = "Error: missing command";
		error->error = true;
		// for(int i = 0; i < args.size; i++ ) {
		// 	free(args.tokens[i]);
		// }
		// free(args.tokens);
		return error;
	}

	if(args.size > 16) {
		struct command* error = malloc(sizeof(struct command));
		error->cmd = "Error: too many process arguments";
		error->error = true;
		// for(int i = 0; i < args.size; i++ ) {
		// 	free(args.tokens[i]);
		// }
		// free(args.tokens);
		return error;
	}

	char** arg = malloc(sizeof(char*)*(args.size + 1));
	char cmd[512] = "";
	char file[512] = "";
	bool isPiped = false;
	bool isRedirected = false;
	int argc = 1;
	struct command* next = NULL;
	
	if (args.tokens[start][0] == '$') {
		if(strlen(args.tokens[start]) > 2 || args.tokens[start][1] > 'z' || args.tokens[start][1] < 'a') {
			struct command* error = malloc(sizeof(struct command));
			error->cmd = "Error: invalid variable name";
			error->error = true;
			// for(int i = 0; i < args.size; i++ ) {
			// 	free(args.tokens[i]);
			// }
			// free(args.tokens);
			return error;
		}
		int index = (int)(args.tokens[start][1] - 'a');
		arg[0] = malloc(sizeof(char)* strlen(args.tokens[start]));
		strcpy(arg[0], sets[index]);
		strcpy(cmd, sets[index]);
		// free(args.tokens[start]);
		
	} else {
		strcpy(cmd, args.tokens[start]);
		arg[0] = args.tokens[start];
	}

	for (int i =  start + 1; i < args.size; i++) {
		if (args.tokens[i][0] == '$') {
			if(strlen(args.tokens[i]) != 2 || args.tokens[i][1] > 'z' || args.tokens[i][1] < 'a') {
				struct command* error = malloc(sizeof(struct command));
				error->cmd = "Error: invalid variable name";
				error->error = true;

				// for(int i = 0; i < args.size; i++ ) {
				// 	free(args.tokens[i]);
				// }

				// free(args.tokens);
				return error;
			}

			int index = (int)(args.tokens[i][1] - 'a');
			arg[argc] = malloc(sizeof(char)* strlen(args.tokens[i]));			
			strcpy(arg[argc], sets[index]);
			// free(args.tokens[i]);
			argc++;
			
		} else if(strcmp(args.tokens[i], ">") == 0) {
			isRedirected = true;
			if(i + 1 < args.size ) {
				strcpy(file, args.tokens[i + 1]);
				i++;
			} else {
				struct command* error = malloc(sizeof(struct command));
				error->cmd = "Error: no output file";
				error->error = true;
				// for(int i = 0; i < args.size; i++ ) {
				// 	free(args.tokens[i]);
				// }

				// free(args.tokens);
				return error;
			}
		} else if (strcmp(args.tokens[i], "|") == 0) {
			isPiped = true;
			if(isRedirected) {
				struct command* error = malloc(sizeof(struct command));
				error->cmd = "Error: mislocated output redirection";
				error->error = true;

				// for(int i = 0; i < args.size; i++ ) {
				// 	free(args.tokens[i]);
				// }

				// free(args.tokens);
				return error;
			} else if(i + 1 < args.size ) {
				next = parse_cmd(args, i + 1, sets);
				if(next->error) {
					return next;
				}
				break;
			} else {
				struct command* error = malloc(sizeof(struct command));
				error->cmd = "Error: missing command";
				error->error = true;

				// for(int i = 0; i < args.size; i++ ) {
				// 	free(args.tokens[i]);
				// }

				// free(args.tokens);
				return error;
			}
		} else {
			arg[argc] = args.tokens[i];
			argc++;
		}
	}
	struct command* c1 = malloc(sizeof(struct command));
	c1->isPiped = isPiped;
	c1->isRedirected = isRedirected;
	c1->cmd = malloc(sizeof(char*)*strlen(cmd));
	strcpy(c1->cmd, cmd);
	c1->next = next;
	if(isRedirected) {
		c1->file = malloc(sizeof(char*)*strlen(file));
		strcpy(c1->file, file);
	}
	arg[argc + 1] = NULL;
	c1->args = arg;
	c1->error = false;

    return (c1);
}

int countPipes(struct command* cmd) {

	int count = 0;
	struct command* next = cmd;
	while(next->next != NULL) {
		count++;
		next = next->next;
	}

	return count;
}
