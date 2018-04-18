#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_LEN 200

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Wrong number of arguments\n");
        exit(1);
    }
    int n = atoi(argv[2]);
    if (n == 0) {
        printf("Wrong last parameter\n");
        exit(1);
    }
    srand(getpid()*time(NULL));
    char buf[MAX_LINE_LEN];
    char buf2[MAX_LINE_LEN];
    printf("Slave PID: %d\n", getpid());
    FILE *pipe = fopen(argv[1], "w");
    for (int i = 0; i < n; i++) {
        FILE *date = popen("date", "r");
        fgets(buf, MAX_LINE_LEN, date);
        sprintf(buf2, "PID: %d Date: %s", getpid(), buf);
        fwrite(buf2, sizeof(char), strlen(buf2), pipe);
        sleep(rand() % 4 + 2);
    }
    fclose(pipe);
}