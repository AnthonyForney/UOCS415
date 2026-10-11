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

#define MAX_FILES 1024

/* Comparison function for qsort() on an array of char* (file names).
 * qsort() passes POINTERS to the array elements, so a and b are really
 * char** and must be dereferenced once to get the strings. */
int compare_files(const void *a, const void *b)
{
    const char *file_a = *(const char **)a;
    const char *file_b = *(const char **)b;

    return strcmp(file_a, file_b);
}

void listDir()
{
    /* TODO 1: Get the current directory with getcwd().
     * Hint: use a char array such as char cwd[1024]. If getcwd() returns
     *       NULL, print an error with perror() and exit. */
    
    char cwd[1024];
    DIR* dir;

    if(getcwd(cwd, sizeof(cwd)) == NULL){
        perror("Unable to reach desired directory");
        exit(1);
    }

    /* TODO 2: Open the directory with opendir().
     * Hint: opendir() returns a DIR*. Check for NULL. */
    if((dir = opendir (".")) == NULL){
        perror("Unable to open .");
        exit(1);
    }
    
    /* A growable array of file names. It starts empty and grows by one
     * slot for each file you keep. */
    char *files[MAX_FILES];
    int file_count = 0;

    /* TODO 3: Read every entry of the directory with readdir().
     * Hint: while ((entry = readdir(dir)) != NULL) { ... }
     *       entry->d_name is the file name.
     *
     * For each name:
     *   a) Skip all unreadable files and directories using stat() and S_ISREG()
     *   b) Skip "output.txt" and "output_reference.txt" using strcmp().
     *   c) Otherwise, add it to files:
     *        - if file_count larger than MAX_FILES, break.
     *        - store a COPY of the name with strdup().
     *      Hint: you need a copy because entry->d_name is overwritten by
     *            the next readdir() call and is gone after closedir(). */
    struct dirent *entry;
    struct stat sb;

    char s1[] = "output.txt";
    char s2[] = "output_reference.txt";
    int file_count = 0;
    
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

    /* TODO 4: Close the directory with closedir(). */

    closedir(dir);

    /* TODO 5: Sort the names with qsort() and compare_files().
     * Hint: qsort(array, number_of_elements, size_of_one_element, compare)
     *       Each element is a char*, so the size is sizeof(char *). */
    
    size_t ElemSize = sizeof(char *);

    qsort(files, file_count, ElemSize, compare_files);

    /* TODO 6: Print the names with write(): each name followed by " ",
     * then one "\n" at the end.
     * Hint: write(STDOUT_FILENO, str, strlen(str)) */
    for(int i = 0; i < file_count, i++){
        write(STDOUT_FILENO, files[i],strlen(files[i]));
        write(STDOUT_FILENO, " ", 1);
    }
    write(STDOUT_FILENO, "\n", 1);

    /* TODO 7: Free the memory.
     * Hint: free each files[i] (from strdup). */
    for(int i; i < file_count; i++){
        free(files[i]);
    }
}
