#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int child_received = 0;
int parent_received = 0;

void sigusr1_c(int signum, siginfo_t *siginfo, void *context) {
    child_received++;
    kill(getppid(), SIGUSR1);
}

void sigusr1_p(int signum, siginfo_t *siginfo, void *context) {
    parent_received++;
}

void sigusr2_c(int signum, siginfo_t *siginfo, void *context) {
    printf("Signals received by child: %d\n",child_received);
    _Exit(1);
}

void sigrtmin_c(int signum, siginfo_t *siginfo, void *context) {
    child_received++;
    kill(getppid(), SIGRTMIN);
}

void sigrtmin_p(int signum, siginfo_t *siginfo, void *context) {
    parent_received++;
}

void sigrtmax_c(int signum, siginfo_t *siginfo, void *context) {
    printf("Signals received by child: %d\n",child_received);
    _Exit(1);
}

int main(int argc, char* argv[]) {
    int signals, type;

    if(argc !=3) {
        printf("Wrong number of arguments\n");
        exit(1);
    }
    
    signals = atoi(argv[1]);
    type = atoi(argv[2]);
    
    if (type<1 || type >3 || signals<=0){
        printf("Wrong parameter\n");
        exit(1);
    }
    
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGUSR2);
    sigaddset(&sigset, SIGRTMIN);
    sigaddset(&sigset, SIGRTMAX);

    pid_t pid = fork();
    struct sigaction sig_act;
    memset(&sig_act, '\0', sizeof(struct sigaction));
    sig_act.sa_mask = sigset;
    sig_act.sa_flags = SA_SIGINFO;

    if(pid == 0) {
        switch (type) {
            case 1:
                sig_act.sa_sigaction = &sigusr1_c;
                sigaction(SIGUSR1, &sig_act, NULL);
                sig_act.sa_sigaction = &sigusr2_c;
                sigaction(SIGUSR2, &sig_act, NULL);
                break;
            case 2:
                sig_act.sa_sigaction = &sigusr1_c;
                sigaction(SIGUSR1, &sig_act, NULL);
                sig_act.sa_sigaction = &sigusr2_c;
                sigaction(SIGUSR2, &sig_act, NULL);
                break;
            case 3:
                sig_act.sa_sigaction = &sigrtmin_c;
                sigaction(SIGRTMIN, &sig_act, NULL);
                sig_act.sa_sigaction = &sigrtmax_c;
                sigaction(SIGRTMAX, &sig_act, NULL);
                break;
            default:
                exit(1);
        }
        while(1);
    } else if(pid > 0) {
        printf("Signals sent by parent: %d\n", signals);
        switch (type) {
            case 1:
                sig_act.sa_sigaction = &sigusr1_p;
                sigaction(SIGUSR1, &sig_act, NULL);
                for(int i = 0; i < signals; i++) {
                    kill(pid, SIGUSR1);
                }
                kill(pid, SIGUSR2);
                break;
            case 2:
                sig_act.sa_sigaction = &sigusr1_p;
                sigaction(SIGUSR1, &sig_act, NULL);
                for(int i = 0; i < signals; i++) {
                    kill(pid, SIGUSR1);
                    pause();
                }
                kill(pid, SIGUSR2);
                break;
            case 3:
                sig_act.sa_sigaction = &sigrtmin_p;
                sigaction(SIGRTMIN, &sig_act, NULL);
                for(int i = 0; i < signals; i++) {
                    kill(pid, SIGRTMIN);
                }
                kill(pid, SIGRTMAX);
                break;
            default:
                exit(1);
        }
        printf("Signals received by parent: %d\n",parent_received);
    } else {
        exit(1);
    }
}