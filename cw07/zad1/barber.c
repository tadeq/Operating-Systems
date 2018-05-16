#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <time.h>
#include "common.h"

struct barbershop *barbershop;
int shm = -1;
int sem = -1;

void handle_signal(int);

void init_semaphores();

void remove_shm_and_semaphores();

void print_message(const char *);

int main(int argc, char **argv) {
    if (argc != 2) FAILURE_EXIT(1, "Barber: Wrong number of arguments (pass only capacity of the waiting room)\n");
    int capacity = strtol(argv[1], NULL, 10);
    if (capacity > MAX_CLIENTS)
        FAILURE_EXIT(1, "Barber: Given capacity is too big. Maximum waiting room capacity is %d\n", MAX_CLIENTS);

    if (atexit(remove_shm_and_semaphores) < 0)
        FAILURE_EXIT(1, "Barber: Can't register atexit function\n");
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    key_t key = ftok(shm_path, PROJECT_ID);
    if ((shm = shmget(key, sizeof(struct barbershop), IPC_CREAT | 0666)) < 0)
        FAILURE_EXIT(1, "Barber: Can't open shared memory\n");
    if ((barbershop = (struct barbershop *) shmat(shm, NULL, 0)) < 0)
        FAILURE_EXIT(1, "Barber: Can't get shared memory address\n");
    barbershop->waiting_room_capacity = capacity;
    barbershop->clients_waiting = 0;
    init_semaphores();
    struct sembuf wait;
    wait.sem_op = -1;
    wait.sem_flg = 0;
    struct sembuf post;
    post.sem_op = 1;
    post.sem_flg = 0;

    char msg[256];
    while (1) {
        wait.sem_num = SEM_ASLEEP;
        semop(sem, &post, 1);
        wait.sem_num = SEM_WAITING_ROOM;
        semop(sem, &post, 1);
        if (barbershop->clients_waiting == 0) {
            post.sem_num = SEM_WAITING_ROOM;
            semop(sem, &post, 1);
            sprintf(msg, "Falling asleep");
            print_message(msg);
            barbershop->barber_asleep = 1;
            post.sem_num = SEM_ASLEEP;
            semop(sem, &post, 1);
            wait.sem_num = SEM_WAKE_UP;
            semop(sem, &wait, 1);
            barbershop->barber_asleep = 0;
            post.sem_num = SEM_ASLEEP;
            semop(sem, &post, 1);
            sprintf(msg, "Waking up");
            print_message(msg);
            post.sem_num = SEM_SIT;
            semop(sem, &post, 1);
            wait.sem_num = SEM_SHAVE;
            semop(sem, &wait, 1);
            sprintf(msg, "Shaving client %d", barbershop->curr_client);
            print_message(msg);
            sprintf(msg, "Ended shaving client %d", barbershop->curr_client);
            print_message(msg);
            post.sem_num = SEM_END_SHAVING;
            semop(sem, &post, 1);
            wait.sem_num = SEM_LEAVE;
            semop(sem, &wait, 1);
        } else {
            sprintf(msg, "Inviting client %d to shave", barbershop->client_pids[0]);
            print_message(msg);
            kill(barbershop->client_pids[0], SIGUSR1);
            post.sem_num = SEM_ASLEEP;
            semop(sem, &post, 1);
            post.sem_num = SEM_SIT;
            semop(sem, &post, 1);
            for (int i = 0; i < barbershop->clients_waiting; ++i)
                barbershop->client_pids[i] = barbershop->client_pids[i + 1];
            barbershop->clients_waiting--;
            post.sem_num = SEM_WAITING_ROOM;
            semop(sem, &post, 1);
            wait.sem_num = SEM_SHAVE;
            semop(sem, &wait, 1);
            sprintf(msg, "Shaving client %d", barbershop->curr_client);
            print_message(msg);
            sprintf(msg, "Ended shaving client %d", barbershop->curr_client);
            print_message(msg);
            post.sem_num = SEM_END_SHAVING;
            semop(sem, &post, 1);
            wait.sem_num = SEM_LEAVE;
            semop(sem, &wait, 1);
        }
    }
    return 0;
}

void handle_signal(int signum) {
    print_message("Closing barbershop");
    exit(0);
}

void init_semaphores() {
    key_t key = ftok(sem_path, PROJECT_ID);
    if ((sem = semget(key, 7, IPC_CREAT | 0666)) < 0)
        FAILURE_EXIT(1, "Barber: Can't get semaphores\n");
    for (int i = 0; i < 2; i++) {
        semctl(sem, i, SETVAL, 1);
    }
    for (int i = 2; i < 7; i++) {
        semctl(sem, i, SETVAL, 0);
    }
}

void remove_shm_and_semaphores() {
    if (shmctl(shm, IPC_RMID, NULL) < 0)
        printf("Barber: Can't remove shared memory\n");
    if (semctl(sem, 0, IPC_RMID, NULL) < 0)
        printf("Barber: Can't remove semaphores\n");
}

void print_message(const char *message) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("%ld.%.6ld", (long) ts.tv_sec, ts.tv_nsec);
    printf(" Barber: %s\n", message);
}