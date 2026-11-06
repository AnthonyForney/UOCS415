#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"
#include <stdbool.h>

#define _GNU_SOURCE

int count_token (char* buf, const char* delim)
{
	
	if(buf == NULL || delim == NULL){
		return 0;
	};

	int delimSize = strlen(delim);

	if(delimSize == 0){
		return 0;
	}

	int tokenCount = 0;

	bool token = false;
	int i = 0;

	while(buf[i] != '\0'){
		if(strncmp(&buf[i], delim, delimSize) == 0){
			token = false;
			i += delimSize;
		}
		
		else{
			if(!token){
			token = true;
			tokenCount++;
		}
		i++;
		}
	}

	return tokenCount;
}

command_line str_filler (char* buf, const char* delim)
{
	command_line cmdline;
	char* saveptr;
	char* saveptr2;

	strtok_r(buf, "\n", &saveptr);

	cmdline.num_token = count_token(buf, delim);
	cmdline.command_list = malloc(sizeof(char*) * (cmdline.num_token + 1));

	char* token = strtok_r(buf, delim, &saveptr2);

	int i = 0;

	while(token != NULL){
		cmdline.command_list[i] = malloc(strlen(token) + 1);
		strcpy(cmdline.command_list[i], token);
		i++;
		token = strtok_r(NULL, delim, &saveptr2);
	}

	cmdline.command_list[i] = NULL; 

	return cmdline;
}

void free_command_line(command_line* command)
{
	if(command == NULL || command->command_list == NULL){
		return;
	}

	for(int i = 0; i < command->num_token; i++){
		free(command->command_list[i]);
	}

	free(command->command_list);

	command->command_list = NULL;
    command->num_token = 0;
}
