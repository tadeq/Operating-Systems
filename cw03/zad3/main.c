#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <sys/time.h>
#include <sys/resource.h>

#define MAX_LINE_LEN 500
#define MAX_ARGS_NO 50

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Too few arguments\n");
        exit(1);
    }
    int time;
    long mem;
    time = atoi(argv[2]);
    mem = atoi(argv[3]);
    mem = mem * 1048576;
    if (time == 0 || mem == 0) {
        printf("Wrong parameter\n");
        exit(1);
    }
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Can't open file\n");
        exit(1);
    }

    struct rlimit time_lim;
    struct rlimit mem_lim;

    time_lim.rlim_cur = time;
    time_lim.rlim_max = time;
    mem_lim.rlim_cur = mem;
    mem_lim.rlim_max = mem;

    struct rusage usage;
    struct rusage tmp;
    getrusage(RUSAGE_CHILDREN, &tmp);

    struct timeval user_time;
    struct timeval system_time;

    const char *delim = " \t\n";
    char buf[MAX_LINE_LEN];
    char *args[MAX_ARGS_NO];
    while (fgets(buf, MAX_LINE_LEN, file) != NULL) {
        int i = 1;
        args[0] = (strtok(buf, delim));
        while ((args[i] = strtok(NULL, delim)) != NULL && i < MAX_ARGS_NO) {
            i++;
        }
        args[i] = NULL;
        pid_t childpid = fork();
        int status;
        if (childpid == 0) {
            if (setrlimit(RLIMIT_CPU, &time_lim) != 0) {
                printf("CPU time limit setting failed\n");
                exit(1);
            }
            if (setrlimit(RLIMIT_AS, &mem_lim) != 0) {
                printf("Virtual memory limit setting failed\n");
                exit(1);
            }
            status = execvp(args[0], args);
            exit(status);
        } else {
            waitpid(childpid, &status, 0);
            if (status != 0) {
                printf("Batch job execution failed\n");
            }
        }
        getrusage(RUSAGE_CHILDREN, &usage);
        timersub(&usage.ru_stime, &tmp.ru_stime, &system_time);
        timersub(&usage.ru_utime, &tmp.ru_utime, &user_time);
        tmp = usage;
        printf("\nBatch job: ");
        for (int j = 0; j < i; j++) {
            printf("%s ", args[j]);
        }
        printf("\nUser time: %d seconds %d microseconds\nSystem time: %d seconds %d microseconds\n\n",
               user_time.tv_sec, user_time.tv_usec, system_time.tv_sec, system_time.tv_usec);
    }
    fclose(file);
}
