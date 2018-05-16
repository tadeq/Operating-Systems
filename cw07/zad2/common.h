#ifndef ZAD2_COMMON_H
#define ZAD2_COMMON_H

#define MAX_CLIENTS 100
#define MAX_SHAVES 10

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}

const char *shm_path = "/shm";
const char *waiting_room = "/waiting_room_sem";
const char *asleep = "/asleep_sem";
const char *wake_up = "/wake_up_sem";
const char *sit = "/sit_sem";
const char *shave = "/shave_sem";
const char *end_shaving = "/end_shaving_sem";
const char *leave = "/leave_sem";

struct barbershop {
    int waiting_room_capacity;
    int client_pids[MAX_CLIENTS];
    int clients_waiting;
    int barber_asleep;
    pid_t curr_client;
};

#endif //ZAD2_COMMON_H
