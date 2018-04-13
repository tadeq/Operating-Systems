#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>

int state_sigtstp = 0;

void sigint(int sig_no) {
    printf("Odebrano sygnał SIGINT\n");
    exit(0);
}

void sigtstp(int sig_no) {
    if (state_sigtstp == 0) {
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTRL+C - zakończenie programu\n");
        state_sigtstp = 1;
    } else
        state_sigtstp = 0;
}

int main() {
    struct sigaction act;
    act.sa_handler = sigtstp;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    time_t sec;

    sigaction(SIGTSTP, &act, NULL);
    signal(SIGINT, sigint);

    while (1) {
        if (state_sigtstp == 0) {
            time(&sec);
            printf("%s", asctime(localtime(&sec)));
        }
        sleep(1);
    }
}