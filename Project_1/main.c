#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "command.h"
#include "string_parser.h"


int main(int argc, char* argv[]){
    size_t len = 1024;
    char *line = malloc(len * sizeof(char));

    /* ------------------------------------------------------------------ */
    /*  Interactive mode                                                   */
    /* ------------------------------------------------------------------ */
    if (argc == 1) {
        while (1) {
            write(STDOUT_FILENO, ">>> ", 4);

            ssize_t lineRead = getline(&line, &len, stdin);
            if(lineRead == -1){
                break;
            }

            command_line cmds;
            const char* delim = ";";
            cmds = str_filler(line, delim);

            for(int i = 0; i < cmds.num_token; i++){
                command_line cmd = str_filler(cmds.command_list[i], " ");
                if(cmd.num_token == 0){
                    free_command_line(&cmd);
                    continue;
                }
            
                //COMMANDS

                if(strcmp(cmd.command_list[0], "ls") == 0){
                    if(cmd.num_token != 1){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        listDir();
                    }
                }

                else if(strcmp(cmd.command_list[0],"pwd") == 0){
                    if(cmd.num_token != 1){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        showCurrentDir();
                    }
                }

                else if(strcmp(cmd.command_list[0], "mkdir") == 0){
                    if(cmd.num_token != 2){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        makeDir(cmd.command_list[1]);
                    }
                }

                else if(strcmp(cmd.command_list[0], "cd") == 0){
                    if(cmd.num_token > 2){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        changeDir(cmd.command_list[1]);
                    }
                }   

                else if(strcmp(cmd.command_list[0], "cp") == 0){
                    if(cmd.num_token != 3){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        copyFile(cmd.command_list[1], cmd.command_list[2]);
                    }
                }

                else if(strcmp(cmd.command_list[0], "mv") == 0){
                    if(cmd.num_token != 3){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        moveFile(cmd.command_list[1], cmd.command_list[2]);
                    }
                }

                else if(strcmp(cmd.command_list[0], "cat") == 0){
                    if(cmd.num_token != 2){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        displayFile(cmd.command_list[1]);
                    }
                }

                else if(strcmp(cmd.command_list[0], "rm") == 0){
                    if(cmd.num_token != 2){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        deleteFile(cmd.command_list[1]);
                    }
                }

                else if(strcmp(cmd.command_list[0], "exit") == 0){
                    free(line);
                    free_command_line(&cmd);
                    free_command_line(&cmds);
                    exit(0);
                }

                else{
                    write(STDOUT_FILENO,"Error! Unrecognized command: ", strlen("Error! Unrecognized command: "));
                    write(STDOUT_FILENO, cmd.command_list[0], strlen(cmd.command_list[0]));
                    write(STDOUT_FILENO, "\n", 1);
                }
                free_command_line(&cmd);
            }
            free_command_line(&cmds);
        }
    }

    /* ------------------------------------------------------------------ */
    /*  File mode                                                          */
    /* ------------------------------------------------------------------ */
    else if (argc == 3 && strcmp(argv[1], "-f") == 0) {
        
        FILE *fin = NULL;

        fin = fopen(argv[2], "r");
    
        if(fin == NULL){
            perror("Failed to open file");
            exit(1);
        }

        if(freopen("output.txt", "w", stdout) == NULL){
            perror("freopen failed!");
            exit(1);
        }
        
        while(getline(&line, &len, fin) != -1){
            command_line cmds;

            const char* delim = ";";
            cmds = str_filler(line, delim);
            
            for(int i = 0; i < cmds.num_token; i++){
                command_line cmd = str_filler(cmds.command_list[i], " ");

                if(cmd.num_token == 0){
                    free_command_line(&cmd);
                    continue;
                }
             
                //COMMANDS
                if(strcmp(cmd.command_list[0], "ls") == 0){
                    if(cmd.num_token != 1){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        listDir();
                    }
                }

                else if(strcmp(cmd.command_list[0],"pwd") == 0){
                    if(cmd.num_token != 1){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        showCurrentDir();
                    }
                }

                else if(strcmp(cmd.command_list[0], "mkdir") == 0){
                    if(cmd.num_token != 2){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        makeDir(cmd.command_list[1]);
                    }
                }

                else if(strcmp(cmd.command_list[0], "cd") == 0){
                    if(cmd.num_token > 2){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        changeDir(cmd.command_list[1]);
                    }
                }   

                else if(strcmp(cmd.command_list[0], "cp") == 0){
                    if(cmd.num_token != 3){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        copyFile(cmd.command_list[1], cmd.command_list[2]);
                    }
                }

                else if(strcmp(cmd.command_list[0], "mv") == 0){
                    if(cmd.num_token != 3){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        moveFile(cmd.command_list[1], cmd.command_list[2]);
                    }
                }

                else if(strcmp(cmd.command_list[0], "cat") == 0){
                    if(cmd.num_token != 2){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        displayFile(cmd.command_list[1]);
                    }
                }

                else if(strcmp(cmd.command_list[0], "rm") == 0){
                    if(cmd.num_token != 2){
                        write(STDOUT_FILENO,"Improper arg count.\n", strlen("Improper arg count.\n"));
                    }
                    else{
                        deleteFile(cmd.command_list[1]);
                    }
                }

                else if(strcmp(cmd.command_list[0], "exit") == 0){
                    free(line);
                    free_command_line(&cmd);
                    free_command_line(&cmds);
                    fclose(fin);
                    exit(0);
                }

                else{
                    write(STDOUT_FILENO,"Error! Unrecognized command: ", strlen("Error! Unrecognized command: "));
                    write(STDOUT_FILENO, cmd.command_list[0], strlen(cmd.command_list[0]));
                    write(STDOUT_FILENO, "\n", 1);
                }
            
                free_command_line(&cmd);
            }
            free_command_line(&cmds);
        }   

        fclose(fin);
    }
    free(line);
    return 0;
}

//TODO
//check for too many or too few parameters
//ls in alphabetical order
//other small bugs
//bad arguments
//run through valgrind
//
