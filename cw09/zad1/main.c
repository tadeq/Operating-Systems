#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define MAX_LINE_LEN 1024

char **buffer;
FILE *input;

int print_all_info;
char comparison_sign;
int prods_no, cons_no, buf_size, len_to_cmp, seconds;
int last_produced = -1, last_consumed = -1, free_places;

int args_good = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t buffer_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t buffer_full = PTHREAD_COND_INITIALIZER;

void clean();

void alarm_handler(int);

int get_args(char*);

void *produce(void*);

void *consume(void*);

int main(int argc, char **argv) {
    if (atexit(clean) != 0) {
        FAILURE_EXIT(1, "Atexit function setting failed\n");
    }
    if (argc < 2) {
        FAILURE_EXIT(1, "Missing config file name\n");
    }
    if (get_args(argv[1]) != 0) {
        FAILURE_EXIT(1, "Wrong value in config file\n");
    } else args_good = 1;
    signal(SIGALRM, alarm_handler);
    buffer = malloc(buf_size * sizeof(char *));
    pthread_t *producers = malloc(prods_no * sizeof(pthread_t));
    pthread_t *consumers = malloc(cons_no * sizeof(pthread_t));
    if (seconds)
        alarm(seconds);
    for (int i = 0; i < prods_no; i++) {
        pthread_create(&producers[i], NULL, produce, NULL);
    }
    for (int i = 0; i < cons_no; i++) {
        pthread_create(&consumers[i], NULL, consume, NULL);
    }
    for (int i = 0; i < prods_no; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < prods_no; i++) {
        pthread_join(consumers[i], NULL);
    }
    free(producers);
    free(consumers);
}

void clean() {
    if (args_good) {
        for (int i = 0; i < buf_size; i++) {
            free(buffer[i]);
        }
        free(buffer);
        fclose(input);
    }
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&buffer_empty);
    pthread_cond_destroy(&buffer_full);
}


void alarm_handler(int signum) {
    if (print_all_info)
        printf("End of time for threads\n");
    exit(0);
}


int get_args(char *filename) {
    /*
     Information in consecutive lines of config file:
     * number of producers
     * number of consumers
     * global buffer size
     * filename to read from
     * string length to compare
     * what lines from buffer to display (<,> or = to value above)
     * 1 - print all information, 0 - only cons_no print when they find matching line
     * number of seconds threads should run (if 0 threads work till EOF or SIGINT or SIGTERM)
    */
    char buf[MAX_LINE_LEN];
    FILE *args = fopen(filename, "r");
    if (args == NULL) {
        FAILURE_EXIT(1, "Args file opening failed\n");
    }
    if (fgets(buf, MAX_LINE_LEN, args) == NULL) return 1;
    prods_no = (int) strtol(buf, NULL, 10);
    if (fgets(buf, MAX_LINE_LEN, args) == NULL) return 1;
    cons_no = (int) strtol(buf, NULL, 10);
    if (fgets(buf, MAX_LINE_LEN, args) == NULL) return 1;
    buf_size = (int) strtol(buf, NULL, 10);
    free_places = buf_size;
    if (fgets(buf, MAX_LINE_LEN, args) == NULL) return 1;
    buf[strcspn(buf, "\n")] = 0;
    input = fopen(buf, "r");
    if (input == NULL) {
        FAILURE_EXIT(1, "Input file opening failed\n");
    }
    if (fgets(buf, MAX_LINE_LEN, args) == NULL) return 1;
    len_to_cmp = (int) strtol(buf, NULL, 10);
    if (fgets(buf, MAX_LINE_LEN, args) == NULL) return 1;
    buf[strcspn(buf, "\n")] = 0;
    if (strcmp(buf, "<") != 0 && strcmp(buf, ">") != 0 && strcmp(buf, "=") != 0) return 1;
    comparison_sign = buf[0];
    if (fgets(buf, MAX_LINE_LEN, args) == NULL) return 1;
    print_all_info = (int) strtol(buf, NULL, 10);
    if (fgets(buf, MAX_LINE_LEN, args) == NULL) return 1;
    seconds = (int) strtol(buf, NULL, 10);
    if (prods_no <= 0 || cons_no <= 0 || buf_size <= 0 || len_to_cmp < 0 ||
        (print_all_info != 0 && print_all_info != 1) || seconds < 0)
        return 1;
    return 0;
}

void *produce(void *arg) {
    char *p_buf;
    while (1) {
        p_buf = malloc(MAX_LINE_LEN * sizeof(char));
        pthread_mutex_lock(&mutex);
        if (!free_places) {
            pthread_cond_wait(&buffer_full, &mutex);
        }
        if (fgets(p_buf, MAX_LINE_LEN, input) == NULL) {
            if (print_all_info)
                printf("End of file to read\n");
            exit(0);
        }
        buffer[++last_produced] = malloc((strlen(p_buf) + 1) * sizeof(char));
        strcpy(buffer[last_produced], p_buf);
        --free_places;
        if (print_all_info) {
            printf("Producer %d inserted new line at index %d\n", (int) pthread_self(), last_produced);
            printf("New line: %s", p_buf);
        }
        if (last_produced == buf_size - 1) last_produced = -1;
        if (free_places > 0) pthread_cond_broadcast(&buffer_empty);
        pthread_mutex_unlock(&mutex);
        free(p_buf);
    }
}

void *consume(void *arg) {
    char *c_buf;
    while (1) {
        c_buf = malloc(MAX_LINE_LEN * sizeof(char));
        pthread_mutex_lock(&mutex);
        while (free_places == buf_size) {
            pthread_cond_wait(&buffer_empty, &mutex);
        }
        strcpy(c_buf, buffer[++last_consumed]);
        if (((strlen(c_buf) - 1) < len_to_cmp && comparison_sign == '<') ||
            ((strlen(c_buf) - 1) == len_to_cmp && comparison_sign == '=') ||
            ((strlen(c_buf) - 1) > len_to_cmp && comparison_sign == '>')) {
            printf("Consumer %d found line with length %c %d at index %d\n", (int) pthread_self(), comparison_sign,
                   len_to_cmp, last_consumed);
            printf("Line: %s\n", c_buf);
        }
        free(buffer[last_consumed]);
        buffer[last_consumed]=NULL;
        ++free_places;
        if (print_all_info)
            printf("Consumer %d erased line at index %d\n", (int) pthread_self(), last_consumed);
        if (last_consumed == buf_size - 1) last_consumed = -1;
        if (free_places < buf_size) pthread_cond_broadcast(&buffer_full);
        pthread_mutex_unlock(&mutex);
        free(c_buf);
    }
}