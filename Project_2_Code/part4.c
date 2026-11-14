/*
 * CIS 415 - Project 2, Part 1: MCP Launches the Workload (MCP v1.0)
 *
 * Usage: ./part1 -f <input_file>
 *
 * Each line of the input file is a command plus its arguments, e.g.
 *     ls -a -r
 *     sleep 1
 *     ./iobound -seconds 10
 *
 * Your MCP v1.0 must:
 *   1. Read every command from the input file.
 *   2. For each command, fork() a child that runs it with execvp().
 *   3. After ALL children have been launched, wait for each one (waitpid).
 *   4. Free all memory, close the file, and exit().
 *
 * Notes on output (the test script compares output character by character):
 *   - Do NOT print extra debug messages in your final submission.
 *   - If execvp() fails, report it with exactly: perror("Execvp");
 *   - Bad arguments must print exactly:
 *         "Invalid use: incorrect number of parameters\n"
 *
 * Always check your code with valgrind:
 *     valgrind --leak-check=full ./part1 -f input.txt
 *
 * Looking ahead: Parts 2, 3, 4 and 5 are completed by EXTENDING this file.
 * Once part1 works (and is valgrind-clean), copy it to part2.c and build on
 * it, then copy part2.c to part3.c, and so on. Keep the code clean and
 * modular now (e.g. a helper that parses a line into args[]) so that later
 * parts are easy to add:
 *   - Part 2: in the child, block SIGUSR1 and sigwait() for it before
 *             execvp(); the parent then sends SIGUSR1, SIGSTOP and SIGCONT
 *             to every child before waiting.
 *   - Part 3: replace "wait for everyone" with a scheduler: use alarm() and
 *             a SIGALRM handler to SIGSTOP the running process and SIGCONT
 *             the next one; track which children have terminated.
 *   - Part 4: each scheduling cycle, read /proc/<pid>/stat (and similar)
 *             for every child and print a top-like table.
 *   - Part 5: (extra credit) adjust each process's time slice based on
 *             whether it looks CPU-bound or I/O-bound.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define MAX_ARGS 50

typedef struct {
    char cpu_state;
    double cpu_sec;
    long vmsize, vmrss;
    long read_b, write_b;
    long utime;
    long stime;
    long nice;
}procInformation;

volatile sig_atomic_t tick = 0;


int read_proc(pid_t pid, procInformation *info);
void on_alarm(int s) {tick = 1;}
void signaler(pid_t* pid, int size, int sig_num);

void print_table(pid_t *pids, int *fin, int n, double *cpu,
                long *rb, long *wb, double elapsed, int cycle);

int main(int argc, char *argv[])
{
    // Check command line arguments: ./part1 -f <input_file>
    if (argc != 3 || strcmp(argv[1], "-f") != 0) {
        printf("Invalid use: incorrect number of parameters\n");
        exit(EXIT_FAILURE);
    }
    
    // Open input file
    FILE *input = fopen(argv[2], "r");
    if (!input) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    
    // Variables for reading the input file with getline()
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    
    // TODO 1: Count the number of lines (commands) in the input file.
    //         Hint: loop over getline() until it returns -1,
    //         then rewind() the file so you can read it again.
    int line_number = 0;
    
    while(getline(&line, &len, input) != -1){
        line_number++;
    }

    
    rewind(input);
    
    // TODO 2: Allocate an array to hold one pid_t per command.
    //         Remember to check the return value of malloc().
    pid_t *pid_array = NULL;
    
    pid_array = malloc(line_number * sizeof(pid_t));
    
    if(pid_array == NULL){
        perror("failed to allocate memory for pid");
        free(line);
        fclose(input);
        exit(EXIT_FAILURE);
    }
    
    int *fin = calloc(line_number, sizeof(int));
    if(fin == NULL){
        perror("Failed to allocate memory");
        free(pid_array);
        free(line);
        fclose(input);
        exit(EXIT_FAILURE);
    }

    double *prev_cpu  = calloc(line_number, sizeof(double));
    long   *prev_rb   = calloc(line_number, sizeof(long));
    long   *prev_wb   = calloc(line_number, sizeof(long));

    if(prev_cpu == NULL || prev_rb == NULL || prev_wb == NULL){
        perror("failed to allocate memory");
        free(pid_array);
        free(fin);
        free(prev_cpu);
        free(prev_rb);
        free(prev_wb);
        free(line);
        fclose(input);
        exit(EXIT_FAILURE);
    }


    int remainder = line_number;
    int curr = 0;
    int cycle = 0;

    struct timespec last_t;
    sigset_t sigset_1, sigset_2;
    int sig;
    
    struct sigaction sa = {0};
    sa.sa_handler = on_alarm;
    sa.sa_flags = SA_NOCLDSTOP;
    if(sigemptyset(&sa.sa_mask) == -1 ||
        sigaction(SIGALRM, &sa, NULL) == -1 ||
        sigaction(SIGCHLD, &sa, NULL) == -1){
            perror("sig error");
            free(line);
            free(fin);
            free(prev_cpu);
            free(prev_rb);
            free(prev_wb);
            free(pid_array);
            fclose(input);
            exit(EXIT_FAILURE);
    }
    
    if(sigemptyset(&sigset_1) == -1 ||
        sigemptyset(&sigset_2) == -1 ||
        sigaddset(&sigset_1, SIGUSR1) == -1 ||
        sigaddset(&sigset_2, SIGALRM) == -1 ||
        sigaddset(&sigset_2, SIGCHLD) == -1 ||
        sigprocmask(SIG_BLOCK, &sigset_1, NULL) == -1 ||
        sigprocmask(SIG_BLOCK, &sigset_2, NULL) == -1){
            perror("sig error");
            free(line);
            free(fin);
            free(prev_cpu);
            free(prev_rb);
            free(prev_wb);
            free(pid_array);
            fclose(input);
            exit(EXIT_FAILURE);
    }

    // Launch one process per line
    for (int i = 0; i < line_number; i++) {
        read = getline(&line, &len, input);
        if (read == -1) {
            break;
        }
        
        // TODO 3: Remove the trailing newline character (if any).
        else{
            if(read > 0 && line[read - 1] == '\n'){
                line[read - 1] = '\0';
            }
        }
        
        // TODO 4: Split the line into tokens (strtok with " ") and store
        //         them in args[]. The array must end with a NULL pointer,
        //         since execvp() expects a NULL-terminated argument list.
        
        char *args[MAX_ARGS];
        int arg_index = 0;
        const char delim[] = " ";
        
        char *token = strtok(line, delim);
        
        while(token != NULL && arg_index < MAX_ARGS - 1){
            args[arg_index++] = token;
            
            token = strtok(NULL, delim);
        }
        
        args[arg_index] = NULL;
        
        // TODO 5: fork() a child process and save its pid in pid_array[i].
        //
        //   - If fork() fails (< 0): report the error with perror(),
        //     free resources, and exit(EXIT_FAILURE).
        //
        //   - In the child (== 0): call execvp(args[0], args).
        //     execvp() only returns on failure. In that case:
        //         perror("Execvp");
        //     then free memory, close the file, and exit(EXIT_FAILURE).
        //     (Why must the child exit here? What happens if it doesn't?)
        //
        //   - In the parent (> 0): do NOT wait here! Continue the loop so
        //     that all processes run at the same time.
        //
        //   (Part 2 hint: this child branch is where the child will later
        //    sigwait() for SIGUSR1 before calling execvp().)
        
        pid_array[i] = fork();
        
        if(pid_array[i] < 0){
            perror("fork failed");
            free(pid_array);
            free(line);
            free(prev_cpu);
            free(prev_rb);
            free(prev_wb);
            free(fin);
            fclose(input);
            exit(EXIT_FAILURE);
        }
        
        else if(pid_array[i] == 0){   
            if(sigwait(&sigset_1, &sig) != 0){
                perror("sigwait");
                free(pid_array);
                free(line);
                free(prev_cpu);
                free(prev_rb);
                free(prev_wb);
                free(fin);
                fclose(input);
                exit(EXIT_FAILURE);
            }

            if(sigprocmask(SIG_UNBLOCK, &sigset_2, NULL) == -1){
                perror("sigprocmask");
                free(pid_array);
                free(line);
                free(prev_cpu);
                free(prev_rb);
                free(prev_wb);
                free(fin);
                fclose(input);
                exit(EXIT_FAILURE);
            }

            if(execvp(args[0], args) == -1){
                perror("Execvp");
                free(pid_array);
                free(line);
                free(prev_cpu);
                free(prev_rb);
                free(prev_wb);
                free(fin);
                fclose(input);
                exit(EXIT_FAILURE);
            } 
        }    
    }
    
    // TODO 6: Now that every process has been launched, wait for each
    //         child to terminate using waitpid() on pid_array[i].
    //
    //   (Part 2/3 hint: the signaling and scheduling logic for later parts
    //    goes here, between launching the children and the final cleanup.)


    
    signaler(pid_array, line_number, SIGUSR1);

	signaler(pid_array, line_number, SIGSTOP);

    printf("MCP: continuing a process %d\n",pid_array[curr]);
    fflush(stdout);

	if(kill(pid_array[0], SIGCONT) == -1){
        perror("failed to kill child");
        free(pid_array);
        free(line);
        free(prev_cpu);
        free(prev_rb);
        free(prev_wb);
        free(fin);
        fclose(input);
        exit(EXIT_FAILURE);
    }
    clock_gettime(CLOCK_MONOTONIC, &last_t);
    alarm(1);
 
    sigset_t empty;
    sigemptyset(&empty);

    while(remainder > 0){
        sigsuspend(&empty);

        for(int i = 0; i < line_number; i++){
            if(!fin[i] && waitpid(pid_array[i], NULL, WNOHANG) > 0){
                fin[i] = 1;
                remainder--;
                printf("MCP: process %d finished\n", pid_array[i]);
                fflush(stdout);
            }
        }

        if(tick || fin[curr]) {
            tick = 0;
            if(!fin[curr]){
                printf("MCP: stopping process %d\n",pid_array[curr]);
                fflush(stdout);
                if(kill(pid_array[curr], SIGSTOP) == -1){
                    perror("failed to kill child");
                    free(pid_array);
                    free(line);
                    free(prev_cpu);
                    free(prev_rb);
                    free(prev_wb);
                    free(fin);
                    fclose(input);
                    exit(EXIT_FAILURE);
                }
            }
            int next = -1;
            for(int j = 1; j <= line_number; j++){
                int k = (curr + j) % line_number;
                if(!fin[k]){
                    next = k;
                    break;
                }
            }
            if(next == -1){
                break;
            }

            int wrapped = (next <= curr);
            curr = next;
            printf("MCP: continuing process %d\n", pid_array[curr]);
            fflush(stdout);
            if(kill(pid_array[curr], SIGCONT) == -1){
                perror("failed to kill child");
                free(pid_array);
                free(line);
                free(prev_cpu);
                free(prev_rb);
                free(prev_wb);
                free(fin);
                fclose(input);
                exit(EXIT_FAILURE);
            }
            if(wrapped){
                struct timespec now;
                clock_gettime(CLOCK_MONOTONIC, &now);
                double elapsed = (now.tv_sec - last_t.tv_sec) +
                                 (now.tv_nsec - last_t.tv_nsec) / 1e9;
                last_t = now;
                print_table(pid_array, fin, line_number, prev_cpu, prev_rb, prev_wb, elapsed, ++cycle);
            }
            alarm(1);
        }
    }
    alarm(0);

    // TODO 7: Clean up: free line and pid_array, close the input file.
    free(line);
    free(pid_array);
    fclose(input);
    free(fin);
    free(prev_cpu);
    free(prev_rb);
    free(prev_wb);
    exit(EXIT_SUCCESS);
}

void signaler(pid_t* pid, int size, int sig_num){

	for(int i = 0; i < size; i++){
       if(kill(pid[i], sig_num) == -1){
			perror("kill failed");
			exit(EXIT_FAILURE);
		}
    }
}

// typedef struct {
//     char cpu_state;
//     double cpu_sec
//     long vmsize, vmrss;
//     long read_b, write_b;
// }procInformation;

int read_proc(pid_t pid, procInformation *p){
    char path[64];
    char buff[1024];
    p->cpu_state = '?';
    p->cpu_sec = 0.0;
    p->vmsize = -1;
    p->vmrss = -1;
    p->read_b = 0;
    p->write_b = 0;

    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    FILE *fp = fopen(path, "r");

    if(fp == NULL){
        return -1;
    }

    if(fgets(buff, sizeof(buff), fp) == NULL){
        fclose(fp);
        return -1;
    }
    fclose(fp);

    long utime = 0;
    long stime = 0;

    sscanf(buff,
           "%*d %*s %c "
           "%*d %*d %*d %*d %*d "
           "%*u %*u %*u %*u %*u "
           "%ld %ld %*d %*d %*d %ld",
           &p->cpu_state, &p->utime, &p->stime, &p->nice);

    


    long tps = sysconf(_SC_CLK_TCK);

    p->cpu_sec = (double)(utime + stime)/ tps;
    

    snprintf(path, sizeof(path),"/proc/%d/status",pid);

    fp = fopen(path, "r");

    if(fp == NULL){
        return -1;
    }

    while(fgets(buff, sizeof(buff), fp) != NULL){
        if(strncmp(buff, "VmSize:", strlen("VmSize:")) == 0){
            sscanf(buff, "VmSize: %ld", &p->vmsize);
        }
        if(strncmp(buff, "VmRSS:", strlen("VmRSS:")) == 0){
            sscanf(buff, "VmRSS: %ld", &p->vmrss);
        }
    }
    fclose(fp);

    snprintf(path, sizeof(path), "/proc/%d/io", pid);

    fp = fopen(path, "r");

    if(fp == NULL){
        return -1;
    }
    while(fgets(buff, sizeof(buff), fp) != NULL){
        if(strncmp(buff, "read_bytes:", strlen("read_bytes:")) == 0){
            sscanf(buff,"read_bytes: %ld", &p->read_b);
        }
        else if(strncmp(buff, "write_bytes:", strlen("write_bytes:")) == 0){
            sscanf(buff, "write_bytes: %ld", &p->write_b);
        }
    }
    fclose(fp);
    return 0;
}   

void print_table(pid_t *pids, int *fin, int n, double *cpu,
                long *rb, long *wb, double elapsed, int cycle){
                
    printf("\n=== Cycle %d complete ===\n", cycle);
    printf("%-7s %-6s %-8s %-7s %-9s %-9s %-8s %-8s %s\n",
           "PID", "STATE", "CPU(s)", "CPU%", "VMRSS(kB)", "VMSIZE(kB)", "WRITE_B(KB)", "READ_B(KB)", "TYPE");
    
    for(int i = 0; i < n; i++){
        procInformation p;

        if (fin[i] || read_proc(pids[i], &p) == -1) {
            printf("%-7d %-6s %-8.2f\n", (int)pids[i], "done", cpu[i]);
            continue;
        }
        double cpu_pct = (elapsed > 0) ? (p.cpu_sec - cpu[i]) / elapsed * 100.0 : 0.0;
        long write_kb = (p.write_b - wb[i]) / 1024;
        long read_kb = (p.read_b - rb[i]) / 1024;

        const char *type = "idle";
        if (cpu_pct >= 50.0){
            type = "CPU-bound";
        }                
        else if (read_kb + write_kb > 0 || cpu_pct >= 5.0){
            type = "I/O-bound";
        }

        printf("%-7d %-6c %-8.2f %-7.1f %-9ld %-9ld %-8ld %-8ld %s\n",
               pids[i], p.cpu_state, p.cpu_sec, cpu_pct, p.vmrss, p.vmsize, write_kb, read_kb, type);

        cpu[i] = p.cpu_sec;
        rb[i]  = p.read_b;
        wb[i] = p.write_b;
    }
    fflush(stdout);
}
