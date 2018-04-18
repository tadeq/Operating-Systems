#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>

#define MAX_SLAVES 100

int slave_pids[MAX_SLAVES];

int main(int argc, char** argv){
    if (argc != 4) {
        printf("Wrong number of arguments\n");
        exit(1);
    }
    int n = atoi(argv[2]);
    if (n == 0) {
        printf("Wrong parameter\n");
        exit(1);
    }
    int slaves = atoi(argv[3]);

    int master_pid=fork();
    if (master_pid==0) {
        execl("./master", "master", argv[1],NULL);
    }
    for (int i=0;i<slaves;i++){
        slave_pids[i]=fork();
            if (slave_pids[i]==0)
                execl("./slave", "slave", argv[1],argv[2],NULL);
    }
    for (int i=0;i<slaves;i++){
        waitpid(slave_pids[i],NULL,WUNTRACED);
    }
    waitpid(master_pid,NULL,WUNTRACED);
}

