#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

volatile int requests;
volatile int requests_needed;
volatile int children;
volatile int running_processes;
pid_t *child_pids;
pid_t *waiting_children;

void rtsig(int signum, siginfo_t *info, void *context) {
    printf("SIGRTMIN+%d received from %d\n", signum - SIGRTMIN, info->si_pid);
}

void request_received(int signum, siginfo_t *info, void *context) {
    printf("SIGUSR1 received from %d\n", info->si_pid);
    if (requests < requests_needed) {
        waiting_children[requests++] = info->si_pid;
        printf("Requests received by parent process: %d\n", requests);
        if (requests >= requests_needed) {
            for (int i = 0; i < requests; ++i) {
                if (waiting_children[i] > 0) {
                    printf("Process %d now can send real time signal\n", waiting_children[i]);
                    kill(waiting_children[i], SIGUSR1);
                    waitpid(waiting_children[i], NULL, 0);
                    running_processes--;
                }
            }
        }
    } else {
        printf("More requests received than needed. Process %d now can send real time signal\n", info->si_pid);
        kill(info->si_pid, SIGUSR1);
        waitpid(info->si_pid, NULL, 0);
        running_processes--;
    }

}

void kill_all(int signum, siginfo_t *info, void *context) {
    for (int i = 0; i < children; i++) {
        if (child_pids[i] > 0) {
            kill(child_pids[i], SIGKILL);
            waitpid(child_pids[i], NULL, 0);
        }
    }
    printf("SIGINT received. All child processes killed\n");
    exit(0);
}

void sigchld(int signum, siginfo_t *info, void *context) {
    printf("Child process %d returned %d\n", info->si_pid, info->si_status);
    if (running_processes == 0) {
        free(waiting_children);
        free(child_pids);
        exit(0);
    }
}

void permission_granted(int signum) {
    kill(getppid(), SIGRTMIN + (rand() % (SIGRTMAX - SIGRTMIN)));
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments\n");
        exit(1);
    }
    children = atoi(argv[1]);
    requests_needed = atoi(argv[2]);
    if (children < requests_needed || children < 1 || requests_needed < 0) {
        printf("Wrong parameters\n");
        exit(1);
    }

    child_pids = calloc((size_t) children, sizeof(int));
    waiting_children = calloc((size_t) children, sizeof(int));

    pid_t pid;
    running_processes = children;

    struct sigaction act;
    act.sa_sigaction = sigchld;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO | SA_NODEFER | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &act, NULL) == -1) {
        printf("SIGCHLD action setting failed\n");
        exit(1);
    }
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = kill_all;
    if (sigaction(SIGINT, &act, NULL) == -1) {
        printf("SIGINT action setting failed\n");
        exit(1);
    }

    act.sa_sigaction = request_received;
    if (sigaction(SIGUSR1, &act, NULL) == -1) {
        printf("SIGUSR1 action setting failed\n");
        exit(1);
    }

    act.sa_sigaction = rtsig;

    for (int i = SIGRTMIN; i <= SIGRTMAX; i++) {
        if (sigaction(i, &act, NULL) == -1) {
            printf("SIGRTMIN+%d action setting failed\n", i - SIGRTMIN);
            exit(1);
        }
    }

    for (int i = 0; i < children; i++) {
        usleep(26210);
        pid = fork();
        if (pid < 0) {
            printf("Fork failed\n");
            exit(1);
        }

        if (pid) {
            printf("Created child process PID: %d\n", pid);
            child_pids[i] = pid;
        } else {
            signal(SIGUSR1, permission_granted);
            sigset_t mask;
            sigfillset(&mask);
            sigdelset(&mask, SIGUSR1);
            uint sleep_time;
            srand((uint) (getppid() * getpid()));
            sleep_time = (uint) rand() % 11;
            sleep(sleep_time);
            kill(getppid(), SIGUSR1);
            sigsuspend(&mask);
            printf("Child %d slept for %d\n", getpid(), sleep_time);
            exit(sleep_time);
        }
    }
    while (wait(NULL));

    return 0;
}
