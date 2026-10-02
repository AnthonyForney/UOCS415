/*
 * string_parser.c
 *
 *  Created on: Nov 25, 2020
 *      Author: gguan, Monil
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"
#include <stdbool.h>

#define _GNU_SOURCE

int count_token (char* buf, const char* delim)
{
	//TODO：
	/*
	*	#1.	Check for NULL string
	*	#2.	iterate through string counting tokens
	*		Cases to watchout for
	*			a.	string start with delimeter
	*			b. 	string end with delimeter
	*			c.	account NULL for the last token
	*	#3. return the number of token (note not number of delimeter)
	*/

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
	//TODO：
	/*
	*	#1.	create command_line variable to be filled and returned
	*	#2.	count the number of tokens with count_token function, set num_token. 
  	*    one can use strtok_r to remove the \n at the end of the line.
	*	#3. malloc memory for token array inside command_line variable
	*			based on the number of tokens.
	*	#4.	use function strtok_r to find out the tokens 
  	*   #5. malloc each index of the array with the length of tokens,
	*			fill command_list array with tokens, and fill last spot with NULL.
	*	#6. return the variable.
	*/

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
	//TODO：
	/*
	*	#1.	free the array base num_token
	*/
	for(int i = 0; i < command->num_token; i++){
		free(command->command_list[i]);
	}
	free(command->command_list);
}
