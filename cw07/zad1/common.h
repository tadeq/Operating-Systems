#ifndef ZAD1_COMMON_H
#define ZAD1_COMMON_H

#define PROJECT_ID 777
#define MAX_CLIENTS 100
#define MAX_SHAVES 10

#define SEM_WAITING_ROOM 0
#define SEM_ASLEEP 1
#define SEM_WAKE_UP 2
#define SEM_SIT 3
#define SEM_SHAVE 4
#define SEM_END_SHAVING 5
#define SEM_LEAVE 6

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

const char *shm_path = "/";
const char *sem_path = "./";

struct barbershop {
    int waiting_room_capacity;
    int client_pids[MAX_CLIENTS];
    int clients_waiting;
    int barber_asleep;
    pid_t curr_client;
};

#endif //ZAD1_COMMON_H
