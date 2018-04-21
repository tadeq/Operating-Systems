#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>


#define MAX_LINE_LEN 500
#define MAX_ARGS_NO 50
#define MAX_PIPES 5

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
    int pipes[2][2];
    const char *delim = " \t\n";
    char buf[MAX_LINE_LEN];
    char *args[MAX_ARGS_NO];
    char *commands[MAX_PIPES+1];
    while (fgets(buf, MAX_LINE_LEN, file) != NULL) {
        int commands_counter = 1;
        delim = "|\n";
        commands[0] = (strtok(buf, delim));
        while ((commands[commands_counter] = strtok(NULL, delim)) != NULL && commands_counter <= MAX_PIPES) {
            commands_counter++;
        }
        commands[commands_counter] = NULL;
        for (int i = 0; i < commands_counter; i++) {
            int args_counter = 1;
            delim = " \t\n";
            char *tmp_com = commands[i];
            args[0] = (strtok(tmp_com, delim));
            while ((args[args_counter] = strtok(NULL, delim)) != NULL && args_counter < MAX_ARGS_NO) {
                args_counter++;
            }
            args[args_counter] = NULL;
            if (i > 1) {
                close(pipes[i % 2][0]);
                close(pipes[i % 2][1]);
            }
            if (pipe(pipes[i % 2]) == -1) {
                printf("Error while creating pipe\n");
                exit(1);
            }
            pid_t childpid = fork();
            if (childpid == 0) {
                if (i > 0) {
                    close(pipes[(i + 1) % 2][1]);
                    dup2(pipes[(i + 1) % 2][0], STDIN_FILENO);
                }
                if (i < commands_counter - 1) {
                    close(pipes[i % 2][0]);
                    dup2(pipes[i % 2][1], STDOUT_FILENO);
                }
                int status = execvp(args[0], args);
                exit(status);
            }
        }
        close(pipes[(commands_counter-1) % 2][0]);
        close(pipes[(commands_counter-1) % 2][1]);
        wait(NULL);
    }
    fclose(file);
}
