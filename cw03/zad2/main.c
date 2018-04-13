#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

#define MAX_LINE_LEN 500
#define MAX_ARGS_NO 50

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Missing name of file containing batch job\n");
        exit(1);
    }
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Can't open file\n");
        exit(1);
    }
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
            status = execvp(args[0], args);
            exit(status);
        } else {
            waitpid(childpid, &status, 0);
            if (status != 0) {
                printf("Batch job execution failed\n");
            }
        }
    }
    fclose(file);
}
