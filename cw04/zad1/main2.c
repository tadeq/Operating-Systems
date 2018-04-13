#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

int state_sigtstp = 0;
int child_pid = 0;
int child_killed = 1;

void sigint(int sig_no) {
    printf("Odebrano sygnał SIGINT\n");
    kill(child_pid, SIGKILL);
    exit(0);
}

void sigtstp(int sig_no) {
    if (!child_killed) {
        printf("Proces potomka zakończony\n");
    } else {
        printf("Nowy proces potomny utworzony\n");
    }
    state_sigtstp = state_sigtstp == 0 ? 1 : 0;
}

int main() {
    struct sigaction act;
    act.sa_handler = sigtstp;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGTSTP, &act, NULL);
    signal(SIGINT, sigint);

    while (1) {
        if (state_sigtstp == 1) {
            if (!child_killed) {
                child_killed = 1;
                kill(child_pid, SIGKILL);
            }
        } else {
            if (child_killed) {
                child_killed = 0;
                child_pid = fork();
                if (child_pid == 0) {
                    execl("date.sh", "date.sh", NULL);
                }
            }
        }
    }
}
