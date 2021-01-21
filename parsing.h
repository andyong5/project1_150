#ifndef project1_150_PARSING_H
#define project1_150_PARSING_H
#include <stdbool.h>
	struct cLine {
		char** tokens;
		int size;
	};

	struct command{
		char* cmd;
		char** args;
		struct command* next;
		bool isPiped;
		bool isRedirected;
		bool error;
		char* file;
	};

	struct cLine parse(char* str);
	struct command* parse_cmd(struct cLine args, int start, char** sets);
	int countPipes(struct command* cmd);
	//void free_cmd(struct command* cmd);

#endif
