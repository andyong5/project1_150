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
  for(int i = 0; i < (int)strlen(str); i++) {
    if (wasEnd && (str[i] == '|' || str[i] == '>')){
      wasEnd = false;
      numEle++;
      /*if(numEle > 16) {
           throw some error
      }*/
      retVal = realloc(retVal, sizeof(char*)*numEle);
      retVal[numEle - 1] = malloc(sizeof(char)*2);
      retVal[numEle - 1][0] = str[i];
      retVal[numEle - 1][1] = '\0';
      curStrLen++;
    } else if (!wasEnd && (str[i] == '|' || str[i] == '>')) {
      retVal[numEle - 1] = realloc(retVal[numEle - 1], (curStrLen + 1) * sizeof(char));
      retVal[numEle - 1][curStrLen] = '\0';
      curStrLen = 0;
      numEle++;
      /*if(numEle > 16) {
           throw some error
      }*/
      retVal = realloc(retVal, sizeof(char*)*numEle);
      retVal[numEle - 1] = malloc(sizeof(char)*2);
      retVal[numEle - 1][0] = str[i];
      retVal[numEle - 1][1] = '\0';
      numEle++;
    } else if(wasEnd && str[i] == ' '){
      continue;
    } else if(wasEnd && str[i] != ' '){
      wasEnd = false;
      numEle++;
      /*if(element > 16) {
           throw some error
      }*/
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
    } else {
    }
  }
  struct cLine ret;
  ret.tokens = retVal;
  ret.size = numEle;

  return ret;
}

struct command* parse_cmd(struct cLine args, int start){
  /*
  if(strcmp(args.tokens[start], ">") {
      throw some error
  }*/
  /*
  if(strcmp(args.tokens[start], "|") {
    throw some error
  }*/

  char arg[512] = "";
  char cmd[512] = "";
  char file[512] = "";
  bool isPiped = false;
  bool isRedirected = false;
  struct command* next = NULL;
  strcpy(cmd, args.tokens[start]);

  for (int i =  start + 1; i < args.size; i++) {
    if(strcmp(args.tokens[i], ">") == 0) {
      isRedirected = true;
      if(i + 1 < args.size ) {
        strcpy(file, args.tokens[i + 1]);
        i++;
      } /*else {
              trow some error
            }*/
    } else if (strcmp(args.tokens[i], "|") == 0) {
      isPiped = true;
      if(i + 1 < args.size ) {
        next = parse_cmd(args, i + 1);
        break;
      } /*else {
                trow some error
            }*/
    } else {
      strcat( arg, " ");
      strcat( arg, args.tokens[i]);
    }
  }
  struct command* c1 = malloc(sizeof(struct command));
  c1->isPiped = isPiped;
  c1->isRedirected = isRedirected;
  c1->cmd = malloc(sizeof(char*)*strlen(cmd));
  c1->args = malloc(sizeof(char*)*strlen(arg));
  strcpy(c1->cmd, cmd);
  strcpy(c1->args, arg);
  c1->next = next;
  if(isRedirected) {
    c1->file = malloc(sizeof(char*)*strlen(file));
    strcpy(c1->file, file);
  }

  if(start == 0) {
    for (int i = 0; i < start; i++) {
      free(args.tokens[i]);
    }
  }

  return (c1);
}