/*
 * command_student.c - CS415 Lab2 starter code
 *
 * Implement listDir(), which is called when the user types "ls".
 *
 * listDir() prints, on ONE line, the names of the files in the current
 * directory that:
 *   - end with ".txt" or ".py",
 *   - start with a digit (0-9),
 *   - are not "output.txt" or "output_reference.txt".
 * The names must be sorted, each followed by one space, and the line
 * ends with '\n'.
 *
 * Example (run from inside files/):
 *   >>> ls
 *   1_poem.txt 2_lyrics.txt 3_DE_Code.py
 *   (there is a space after every name, including the last one)
 *
 * Requirements:
 *   - Use write(STDOUT_FILENO, ...) for output, not printf(). In file
 *     mode, stdout is redirected to output.txt, and write() goes straight
 *     to that file.
 *   - Your program must pass valgrind with NO memory errors and with
 *     "All heap blocks were freed". Every malloc/strdup needs a
 *     free(), and every opendir() needs a closedir().
 *   - Run ./checker.sh to test your work.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_FILES 1024

/* Comparison function for qsort() on an array of char* (file names).
 * qsort() passes POINTERS to the array elements, so a and b are really
 * char** and must be dereferenced once to get the strings. */
int compare_files(const void *a, const void *b)
{
    const char *file_a = *(const char **)a;
    const char *file_b = *(const char **)b;

    return strcasecmp(file_a, file_b);
}

void listDir()
{   
    char cwd[1024];
    DIR* dir;

    if(getcwd(cwd, sizeof(cwd)) == NULL){
        perror("Unable to reach desired directory");
        exit(1);
    }

    if((dir = opendir (".")) == NULL){
        perror("Unable to open directory");
        exit(1);
    }
    
    char *files[MAX_FILES];
    int file_count = 0;

    struct dirent *entry;
    struct stat sb;

    char s1[] = "output.txt";
    char s2[] = "output_reference.txt";
    
    while((entry = readdir(dir)) != NULL) {
        if(stat(entry->d_name,&sb) == -1 ||
            !S_ISREG(sb.st_mode)){
            continue;
        }
     
        if(strcmp(entry->d_name,s1) == 0||
            strcmp(entry->d_name,s2) == 0){
            continue;
        }
        
        if(file_count >= MAX_FILES){
            break;
        }
        
        char *duplicate = strdup(entry->d_name);

        if(duplicate == NULL){
            perror("Failed to allocate memory");
            exit(1);
        }

        files[file_count] = duplicate;
        file_count++;
    }

    closedir(dir);

    size_t ElemSize = sizeof(char *);

    qsort(files, file_count, ElemSize, compare_files);

    for(int i = 0; i < file_count; i++){
        write(STDOUT_FILENO, files[i],strlen(files[i]));
        write(STDOUT_FILENO, " ", 1);
    }

    write(STDOUT_FILENO, "\n", 1);

    for(int i = 0; i < file_count; i++){
        free(files[i]);
    }
}

void showCurrentDir(){
    char *cwd = getcwd(NULL, 0);

    if(cwd){
        write(STDOUT_FILENO, cwd, strlen(cwd));
        write(STDOUT_FILENO, "\n", 1);
    }
    else{
        write(STDOUT_FILENO, "Couldn't find your current Directory.\n",
        strlen( "Couldn't find your current Directory.\n"));
        exit(1);
    }
    free(cwd);
}

void makeDir(char *dirName){
    if(strchr(dirName, '\\') != NULL){
        write(STDOUT_FILENO,"Error: '\\' is not allowed in file name.\n",
            strlen("Error: '\\' is not allowed in file name.\n"));
        //write(STDOUT_FILENO, "\n", 1);
        return;
    }
    
    if(mkdir(dirName, 0777) == -1){
        if(errno == EEXIST){
            write(STDOUT_FILENO, "File name ", strlen("File name "));
            write(STDOUT_FILENO, dirName, strlen(dirName));
            write(STDOUT_FILENO, " already exists.\n", strlen(" already exists.\n"));
           // write(STDOUT_FILENO, "\n", 1);
        }
        else{
            write(STDOUT_FILENO, "error making new directory.", 
            strlen("error making new directory."));
        }
    }
}
// pssible errors:
//file exists(same name)
//file name has forbidden characters

void changeDir(char *dirName){
    
    if(dirName == NULL){
        chdir(getenv("HOME"));
        showCurrentDir();
    }

    else{
        chdir(dirName);
        showCurrentDir();
    }
    // cd: returns to home directory
    //cd .. returns to parent directory
    //cd <dirname> changes to dirname
}

void copyFile(char *sourcePath, char *destinationPath){
    //check input =/= output
    //open file
    //read file
    //copy it to a buffer
    //write it to destination

struct stat st, src_st, tgt_st;
    
    if(stat(sourcePath, &src_st) == -1 || !S_ISREG(src_st.st_mode)){
        write(STDOUT_FILENO,"unable to find ", strlen("unable to find "));
        write(STDOUT_FILENO, sourcePath, strlen(sourcePath));
        write(STDOUT_FILENO, ".\n", 2);
        return;
    }
    
    int dest_existence = (stat(destinationPath, &st) == 0);

    size_t dpl = strlen(destinationPath);

    if(!dest_existence && dpl > 0 && destinationPath[dpl - 1] == '/'){
        write(STDOUT_FILENO, "unable to find dir: ", strlen("unable to find dir: "));
        write(STDOUT_FILENO, destinationPath, strlen(destinationPath));
        write(STDOUT_FILENO, ".\n", 2);
        return;
    }

    const char *target = destinationPath;
    char *newPath = NULL;

    if(dest_existence && S_ISDIR(st.st_mode)){
        const char *slash = strrchr(sourcePath, '/');
        const char *name = (slash != NULL) ? slash + 1 : sourcePath;

        newPath = malloc(strlen(destinationPath) + 1 + strlen(name) + 1);
        if(newPath == NULL){
            return;
        }

        strcpy(newPath, destinationPath);
        strcat(newPath, "/");
        strcat(newPath, name);
        target = newPath;
    }

    if(stat(sourcePath, &src_st) == 0 && 
        stat(target, &tgt_st) == 0 &&
        src_st.st_dev == tgt_st.st_dev &&
        src_st.st_ino == tgt_st.st_ino){

        write(STDOUT_FILENO, "Error: Files share the same name!\n", 
        strlen("Error: Files share the same name!\n"));
        free(newPath);
        return;
    }

    int sp = open(sourcePath, O_RDONLY);
    if(sp == -1){
        write(STDOUT_FILENO,"unable to find ", strlen("unable to find "));
        write(STDOUT_FILENO, sourcePath, strlen(sourcePath));
        write(STDOUT_FILENO, ".\n", 2);
        return;
    }

    int dp = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if(dp == -1){
        int saved = errno;
        close(sp);
        free(newPath);
        
        if(saved == ENOENT){
            write(STDOUT_FILENO,"unable to find ", strlen("unable to find "));
            write(STDOUT_FILENO, target, strlen(target));
            write(STDOUT_FILENO, ".\n", 2);
        }

        else{
            write(STDOUT_FILENO,"unable to copy ", strlen("unable to copy "));
            write(STDOUT_FILENO, sourcePath, strlen(sourcePath));
            write(STDOUT_FILENO," to ", 4);
            write(STDOUT_FILENO, target, strlen(target));
            write(STDOUT_FILENO, ".\n", 2);
        }
    }

    char buffer[4096];
    ssize_t n;

    while((n = read(sp, buffer, sizeof(buffer))) > 0){
        write(dp, buffer, n);
    }

    close(sp);
    close(dp);
    free(newPath);
}

void moveFile(char *sourcePath, char *destinationPath){
    //just cp but deleting the source     
    //was more than just that btw
    //will create file but not directory
    struct stat st, src_st, tgt_st;
    
    if(stat(sourcePath, &src_st) == -1 || !S_ISREG(src_st.st_mode)){
        write(STDOUT_FILENO,"unable to find ", strlen("unable to find "));
        write(STDOUT_FILENO, sourcePath, strlen(sourcePath));
        write(STDOUT_FILENO, ".\n", 2);
        return;
    }
    
    int dest_existence = (stat(destinationPath, &st) == 0);

    size_t dpl = strlen(destinationPath);

    if(!dest_existence && dpl > 0 && destinationPath[dpl - 1] == '/'){
        write(STDOUT_FILENO, "unable to find dir: ", strlen("unable to find dir: "));
        write(STDOUT_FILENO, destinationPath, strlen(destinationPath));
        write(STDOUT_FILENO, ".\n", 2);
        return;
    }

    const char *target = destinationPath;
    char *newPath = NULL;

    if(dest_existence && S_ISDIR(st.st_mode)){
        const char *slash = strrchr(sourcePath, '/');
        const char *name = (slash != NULL) ? slash + 1 : sourcePath;

        newPath = malloc(strlen(destinationPath) + 1 + strlen(name) + 1);

        if(newPath == NULL){
            return;
        }
        strcpy(newPath, destinationPath);
        strcat(newPath, "/");
        strcat(newPath, name);
        target = newPath;
    }

    if(stat(sourcePath, &src_st) == 0 && 
        stat(target, &tgt_st) == 0 &&
        src_st.st_dev == tgt_st.st_dev &&
        src_st.st_ino == tgt_st.st_ino){

        write(STDOUT_FILENO, "Error: Files share the same name!\n", 
        strlen("Error: Files share the same name!\n"));
        free(newPath);
        return;
    }

    int sp = open(sourcePath, O_RDONLY);
    if(sp == -1){
        write(STDOUT_FILENO,"unable to find ", strlen("unable to find "));
        write(STDOUT_FILENO, sourcePath, strlen(sourcePath));
        write(STDOUT_FILENO, ".\n", 2);
        return;
    }

    int dp = open(target, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if(dp == -1){
        int saved = errno;
        close(sp);
        free(newPath);
        
        if(saved == ENOENT){
            write(STDOUT_FILENO,"unable to find ", strlen("unable to find "));
            write(STDOUT_FILENO, target, strlen(target));
            write(STDOUT_FILENO, ".\n", 2);
        }

        else{
            write(STDOUT_FILENO,"unable to move ", strlen("unable to move "));
            write(STDOUT_FILENO, sourcePath, strlen(sourcePath));
            write(STDOUT_FILENO," to ", 4);
            write(STDOUT_FILENO, target, strlen(target));
            write(STDOUT_FILENO, ".\n", 2);
        }
    }

    char buffer[4096];
    ssize_t n;

    while((n = read(sp, buffer, sizeof(buffer))) > 0){
        write(dp, buffer, n);
    }

    close(sp);
    close(dp);
    free(newPath);

    if(n == 0 && unlink(sourcePath)== -1){
        write(STDOUT_FILENO,"unable to delete ", strlen("unable to delete "));
        write(STDOUT_FILENO, sourcePath, strlen(sourcePath));
        write(STDOUT_FILENO, ".\n",2);
    }
}

void deleteFile(char *filename){
    struct stat st;
    
    if(stat(filename, &st) == -1 || !S_ISREG(st.st_mode)){
        write(STDOUT_FILENO,"unable to find ", strlen("unable to find "));
        write(STDOUT_FILENO, filename, strlen(filename));
        write(STDOUT_FILENO, ".\n", 2);
        return;
    }

    if(unlink(filename) == -1){
        write(STDOUT_FILENO, "unable to delete ",strlen("unable to delete "));
        write(STDOUT_FILENO, filename, strlen(filename));
        write(STDOUT_FILENO, ".\n", 2);
    }   
}

void displayFile(char *filename){
    struct stat st;
    
    if(filename == NULL || stat(filename, &st) == -1 || !S_ISREG(st.st_mode)){
        write(STDOUT_FILENO,"unable to find ", strlen("unable to find "));
        write(STDOUT_FILENO, filename, strlen(filename));
        write(STDOUT_FILENO, ".\n", 2);
        return;
    }

    int fn = open(filename, O_RDONLY);
    if(fn == -1){
        write(STDOUT_FILENO,"unable to open ", strlen("unable to open "));
        write(STDOUT_FILENO, filename, strlen(filename));
        write(STDOUT_FILENO, ".\n", 2);
        return;
    }

    char buffer[4096];
    ssize_t n;

    while((n = read(fn, buffer, sizeof(buffer))) > 0){
        write(STDOUT_FILENO, buffer, n);
    }

    close(fn);
}