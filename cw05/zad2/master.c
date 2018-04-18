#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

#define MAX_LINE_LEN 200

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Wrong number of arguments\n");
        exit(1);
    }
    if (mkfifo(argv[1], S_IREAD | S_IWRITE) == -1) {
        printf("Error when creating fifo\n");
        exit(1);
    }
    char buf[MAX_LINE_LEN];
    FILE *pipe = fopen(argv[1], "r");
    while (fgets(buf, MAX_LINE_LEN, pipe) != NULL) {
        printf("%s\n", buf);
    }
    fclose(pipe);
}
