#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>

#include "common.h"

int first_for_shaving = 0;

struct barbershop *barbershop;
int shm = -1;

sem_t *sem_waiting_room;
sem_t *sem_asleep;
sem_t *sem_wake_up;
sem_t *sem_sit;
sem_t *sem_shave;
sem_t *sem_end_shaving;
sem_t *sem_leave;

void handle_exit(int);

void handle_invitation(int);

sem_t *open_semaphore(sem_t *, const char *);

void init_semaphores();

int semaphore_close(sem_t *, const char *);

void remove_semaphores();

void print_message(const char *);

int main(int argc, char **argv) {
    if (atexit(remove_semaphores) < 0)
        FAILURE_EXIT(1, "Client: Can't register atexit function\n");
    signal(SIGTERM, handle_exit);
    signal(SIGINT, handle_exit);
    signal(SIGUSR1, handle_invitation);
    if (argc != 3)
        FAILURE_EXIT(1, "Client: Wrong number of arguments (pass number of clients and how many times they should be shaved)\n");
    int no_of_clients = strtol(argv[1], NULL, 10);
    int no_of_shaves = strtol(argv[2], NULL, 10);
    if (no_of_clients > MAX_CLIENTS)
        FAILURE_EXIT(1, "Client: Too many clients to create. Maximum number of clients is %d\n", MAX_CLIENTS);
    if (no_of_shaves > MAX_SHAVES)
        FAILURE_EXIT(1, "Client: Number of shaves too big. One client can be shaved only %d times\n", MAX_SHAVES)
    init_semaphores();
    int shm = -1;
    if ((shm = shm_open(shm_path, 0666, DEFFILEMODE)) < 0) FAILURE_EXIT(1, "Client: Can't open shared memory\n");
    if ((barbershop = (struct barbershop *) mmap(NULL, sizeof(struct barbershop), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0)) < 0)
        FAILURE_EXIT(1, "Client: Can't get shared memory address\n");

    pid_t clients[MAX_CLIENTS];
    int clients_counter = 0;
    for (int i = 0; i < no_of_clients; i++) {
        pid_t client_pid = fork();
        if (client_pid == 0) {
            char msg[256];
            for (int j = 0; j < no_of_shaves; j++) {
                sem_wait(sem_asleep);
                if (barbershop->barber_asleep == 1) {
                    barbershop->curr_client = getpid();
                    sprintf(msg, "Waking the barber up");
                    print_message(msg);
                    sem_post(sem_wake_up);
                    sem_wait(sem_sit);
                    sprintf(msg, "Sitting on a barber's chair");
                    print_message(msg);
                    sem_post(sem_shave);
                    sem_wait(sem_end_shaving);
                    sprintf(msg, "Shaved for the %d time", j + 1);
                    print_message(msg);
                    if (j + 1 < no_of_shaves)
                        sprintf(msg, "Leaving barbershop, will be back");
                    else
                        sprintf(msg, "Leaving barbershop");
                    print_message(msg);
                    sem_post(sem_leave);
                } else {
                    sem_wait(sem_waiting_room);
                    if (barbershop->clients_waiting >= barbershop->waiting_room_capacity) {
                        sprintf(msg, "No free places in waiting room. Leaving barbershop, will be back");
                        print_message(msg);
                        sem_post(sem_asleep);
                        sem_post(sem_waiting_room);
                    } else {
                        sprintf(msg, "Barber is busy. Taking seat in the queue");
                        print_message(msg);
                        barbershop->client_pids[barbershop->clients_waiting] = getpid();
                        barbershop->clients_waiting++;
                        sem_post(sem_asleep);
                        sem_post(sem_waiting_room);
                        while (!first_for_shaving);
                        first_for_shaving = 0;
                        sem_wait(sem_sit);
                        sprintf(msg, "Sitting on a barber's chair");
                        print_message(msg);
                        barbershop->curr_client = getpid();
                        sem_post(sem_shave);
                        sem_wait(sem_end_shaving);
                        sprintf(msg, "Shaved for the %d time", j + 1);
                        print_message(msg);
                        if (j + 1 < no_of_shaves)
                            sprintf(msg, "Leaving barbershop, will be back");
                        else
                            sprintf(msg, "Leaving barbershop");
                        print_message(msg);
                        sem_post(sem_leave);
                    }
                }
            }
            return 0;
        } else {
            clients[clients_counter] = client_pid;
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

sem_t *open_semaphore(sem_t *sem, const char *name) {
    if ((sem = sem_open(name, 0666)) < 0)
        FAILURE_EXIT(1, "Client: Can't open semaphore %s\n", name);
    return sem;
}

void init_semaphores() {
    sem_waiting_room = open_semaphore(sem_waiting_room, waiting_room);
    sem_asleep = open_semaphore(sem_asleep, asleep);
    sem_wake_up = open_semaphore(sem_wake_up, wake_up);
    sem_sit = open_semaphore(sem_sit, sit);
    sem_shave = open_semaphore(sem_shave, shave);
    sem_end_shaving = open_semaphore(sem_end_shaving, end_shaving);
    sem_leave = open_semaphore(sem_leave, leave);
}

int semaphore_close(sem_t *sem, const char *name) {
    int res;
    if (res = sem_close(sem) < 0) {
        printf("Client: Can't close semaphore %s\n", name);
        return res;
    }
    return 0;
}

void remove_semaphores() {
    semaphore_close(sem_waiting_room, waiting_room);
    semaphore_close(sem_asleep, asleep);
    semaphore_close(sem_wake_up, wake_up);
    semaphore_close(sem_sit, sit);
    semaphore_close(sem_shave, shave);
    semaphore_close(sem_end_shaving, end_shaving);
    semaphore_close(sem_leave, leave);
}

void print_message(const char *message) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("%ld.%.6ld", (long) ts.tv_sec, ts.tv_nsec);
    printf(" Client %d: %s\n", getpid(), message);
}