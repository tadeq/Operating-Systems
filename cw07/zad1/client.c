#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>

#include "common.h"

int first_for_shaving = 0;

struct barbershop *barbershop;
int shm = -1;

void handle_exit(int);

void handle_invitation(int);

void print_message(const char *);

int main(int argc, char **argv) {
    if (argc != 3)
        FAILURE_EXIT(1, "Client: Wrong number of arguments (pass number of clients and how many times they should be shaved)\n");
    int no_of_clients = strtol(argv[1], NULL, 10);
    int no_of_shaves = strtol(argv[2], NULL, 10);
    if (no_of_clients > MAX_CLIENTS)
        FAILURE_EXIT(1, "Client: Too many clients to create. Maximum number of clients is %d\n", MAX_CLIENTS);
    if (no_of_shaves > MAX_SHAVES)
        FAILURE_EXIT(1, "Client: Number of shaves too big. One client can be shaved only %d times\n", MAX_SHAVES);

    signal(SIGTERM, handle_exit);
    signal(SIGINT, handle_exit);
    signal(SIGUSR1, handle_invitation);
    key_t sem_key = ftok(sem_path, PROJECT_ID);
    int sem;
    if ((sem = semget(sem_key, 0, 0666)) < 0)
        FAILURE_EXIT(1, "Client: Can't get semaphores\n");
    key_t shm_key = ftok(shm_path, PROJECT_ID);
    int shm;
    if ((shm = shmget(shm_key, sizeof(struct barbershop), 0666)) < 0)
        FAILURE_EXIT(1, "Client: Can't open shared memory\n");
    if ((barbershop = (struct barbershop *) shmat(shm, NULL, 0)) < 0)
        FAILURE_EXIT(1, "Client: Can't get shared memory address\n");

    struct sembuf wait;
    wait.sem_op = -1;
    wait.sem_flg = 0;
    struct sembuf post;
    post.sem_op = 1;
    post.sem_flg = 0;
    pid_t clients[MAX_CLIENTS];
    int clients_counter = 0;
    for (int i = 0; i < no_of_clients; i++) {
        pid_t child = fork();
        if (child == 0) {
            char msg[256];
            for (int j = 0; j < no_of_shaves; j++) {
                wait.sem_num = SEM_ASLEEP;
                semop(sem, &wait, 1);
                if (barbershop->barber_asleep == 1) {
                    barbershop->curr_client = getpid();
                    sprintf(msg, "Waking the barber up");
                    print_message(msg);
                    post.sem_num = SEM_WAKE_UP;
                    semop(sem, &post, 1);
                    wait.sem_num = SEM_SIT;
                    semop(sem, &wait, 1);
                    sprintf(msg, "Sitting on a barber's chair");
                    print_message(msg);
                    post.sem_num = SEM_SHAVE;
                    semop(sem, &post, 1);
                    wait.sem_num = SEM_END_SHAVING;
                    semop(sem, &wait, 1);
                    sprintf(msg, "Shaved for the %d time", j + 1);
                    print_message(msg);
                    if (j + 1 < no_of_shaves)
                        sprintf(msg, "Leaving barbershop, will be back");
                    else
                        sprintf(msg, "Leaving barbershop");
                    print_message(msg);
                    post.sem_num = SEM_LEAVE;
                    semop(sem, &post, 1);
                } else {
                    wait.sem_num = SEM_WAITING_ROOM;
                    semop(sem, &wait, 1);
                    if (barbershop->clients_waiting >= barbershop->waiting_room_capacity) {
                        sprintf(msg, "No free places in waiting room. Leaving barbershop, will be back");
                        print_message(msg);
                        post.sem_num = SEM_ASLEEP;
                        semop(sem, &post, 1);
                        post.sem_num = SEM_WAITING_ROOM;
                        semop(sem, &post, 1);
                    } else {
                        sprintf(msg, "Barber is busy. Taking seat in the queue");
                        print_message(msg);
                        barbershop->client_pids[barbershop->clients_waiting] = getpid();
                        barbershop->clients_waiting++;
                        post.sem_num = SEM_ASLEEP;
                        semop(sem, &post, 1);
                        post.sem_num = SEM_WAITING_ROOM;
                        semop(sem, &post, 1);
                        while (!first_for_shaving);
                        first_for_shaving = 0;
                        wait.sem_num = SEM_SIT;
                        semop(sem, &wait, 1);
                        sprintf(msg, "Sitting on a barber's chair");
                        print_message(msg);
                        barbershop->curr_client = getpid();
                        post.sem_num = SEM_SHAVE;
                        semop(sem, &post, 1);
                        wait.sem_num = SEM_END_SHAVING;
                        semop(sem, &wait, 1);
                        sprintf(msg, "Shaved for the %d time", j + 1);
                        print_message(msg);
                        if (j + 1 < no_of_shaves)
                            sprintf(msg, "Leaving barbershop, will be back");
                        else
                            sprintf(msg, "Leaving barbershop");
                        print_message(msg);
                        post.sem_num = SEM_LEAVE;
                        semop(sem, &post, 1);
                    }
                }
            }
            return 0;
        } else {
            clients[clients_counter] = child;
            clients_counter++;
        }
    }
    for (int k = 0; k < clients_counter; k++) {
        waitpid(clients[k], NULL, 0);
    }
    return 0;
}

void handle_exit(int signum) {
    print_message("Leaving barbershop");
    exit(0);
}

void handle_invitation(int signum) {
    print_message("First for shaving");
    first_for_shaving = 1;
}

void print_message(const char *message) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("%ld.%.6ld", (long) ts.tv_sec, ts.tv_nsec);
    printf(" Client %d: %s\n", getpid(), message);
}