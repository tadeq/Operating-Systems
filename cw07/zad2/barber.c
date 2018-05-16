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

#include "common.h"

struct barbershop *barbershop;
int shm = -1;

sem_t *sem_waiting_room;
sem_t *sem_asleep;
sem_t *sem_wake_up;
sem_t *sem_sit;
sem_t *sem_shave;
sem_t *sem_end_shaving;
sem_t *sem_leave;

void handle_signal(int);

sem_t *open_semaphore(sem_t *, const char *);

void init_semaphores();

int sem_close_unlink(sem_t *, const char *);

void remove_shm_and_semaphores();

void print_message(const char *);

int main(int argc, char **argv) {
    if (argc != 2)
        FAILURE_EXIT(1, "Barber: Wrong number of arguments (pass only capacity of the waiting room)\n");
    int capacity = strtol(argv[1], NULL, 10);
    if (capacity>MAX_CLIENTS)
        FAILURE_EXIT(1, "Barber: Given capacity is too big. Maximum waiting room capacity is %d",MAX_CLIENTS);
    if (atexit(remove_shm_and_semaphores) < 0)
        FAILURE_EXIT(1, "Barber: Can't register atexit function\n");

    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    init_semaphores();
    if ((shm = shm_open(shm_path, O_CREAT | O_RDWR | O_TRUNC, DEFFILEMODE)) < 0)
        FAILURE_EXIT(1, "Barber: Can't open shared memory\n");
    ftruncate(shm, sizeof(struct barbershop));
    if ((barbershop = (struct barbershop *) mmap(NULL, sizeof(struct barbershop), PROT_READ | PROT_WRITE, MAP_SHARED, shm, 0)) < 0)
        FAILURE_EXIT(1, "Barber: Can't get shared memory address\n");
    barbershop->waiting_room_capacity = capacity;
    barbershop->clients_waiting = 0;
    for (int i = 0; i < capacity; i++) {
        barbershop->client_pids[i] = 0;
    }

    char msg[256];
    while (1) {
        sem_wait(sem_asleep);
        sem_wait(sem_waiting_room);
        sprintf(msg, "%d clients in waiting room", barbershop->clients_waiting);
        print_message(msg);
        if (barbershop->clients_waiting == 0) {
            sem_post(sem_waiting_room);
            sprintf(msg, "Falling asleep");
            print_message(msg);
            barbershop->barber_asleep = 1;
            sem_post(sem_asleep);
            sem_wait(sem_wake_up);
            barbershop->barber_asleep = 0;
            sem_post(sem_asleep);
            sprintf(msg, "Waking up");
            print_message(msg);
            sem_post(sem_sit);
            sem_wait(sem_shave);
            sprintf(msg, "Shaving client %d", barbershop->curr_client);
            print_message(msg);
            sprintf(msg, "Ended shaving client %d", barbershop->curr_client);
            print_message(msg);
            sem_post(sem_end_shaving);
            sem_wait(sem_leave);
        } else {
            sprintf(msg, "Inviting client %d to shave", barbershop->client_pids[0]);
            print_message(msg);
            kill(barbershop->client_pids[0], SIGUSR1);
            sem_post(sem_asleep);
            sem_post(sem_sit);
            for (int i = 0; i < barbershop->clients_waiting; i++)
                barbershop->client_pids[i] = barbershop->client_pids[i + 1];
            barbershop->clients_waiting--;
            sem_post(sem_waiting_room);
            sem_wait(sem_shave);
            sprintf(msg, "Shaving client %d", barbershop->curr_client);
            print_message(msg);
            sprintf(msg, "Ended shaving client %d", barbershop->curr_client);
            print_message(msg);
            sem_post(sem_end_shaving);
            sem_wait(sem_leave);
        }
    }
    return 0;
}

void handle_signal(int signum) {
    print_message("Closing barbershop");
    exit(0);
}

sem_t *open_semaphore(sem_t *sem, const char *name) {
    if ((sem = sem_open(name, O_CREAT | 0666, DEFFILEMODE, 1)) < 0)
        FAILURE_EXIT(1, "Barber: Can't open semaphore %s\n", name);
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

int sem_close_unlink(sem_t *sem, const char *name) {
    int res;
    if (res = sem_close(sem) < 0) {
        printf("Barber: Can't close semaphore %s\n", name);
        return res;
    }
    if (res = sem_unlink(name) < 0) {
        printf("Barber: Can't unlink semaphore %s\n", name);
        return res;
    }
    return 0;
}

void remove_shm_and_semaphores() {
    if(shm_unlink(shm_path) < 0) printf("Barber: Can't unlink shared memory\n");
    sem_close_unlink(sem_waiting_room, waiting_room);
    sem_close_unlink(sem_asleep, asleep);
    sem_close_unlink(sem_wake_up, wake_up);
    sem_close_unlink(sem_sit, sit);
    sem_close_unlink(sem_shave, shave);
    sem_close_unlink(sem_end_shaving, end_shaving);
    sem_close_unlink(sem_leave, leave);
}

void print_message(const char *message) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("%ld.%.6ld", (long) ts.tv_sec, ts.tv_nsec);
    printf(" Barber: %s\n", message);
}