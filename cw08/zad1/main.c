#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <pthread.h>

#define FAILURE_EXIT(code, format, ...) { fprintf(stderr, format, ##__VA_ARGS__); exit(code);}
#define MAX_LINE_LEN 16384

int **image;
double **filter;
int **result;

int width, height, filter_size, max_pix_val;
int threads_num;

void free_all();

void load_image(char *);

void load_filter(char *);

void save_result(char *);

double calculate_pixel(int, int);

void *filter_image(void *);

void take_measurements(struct timeval *, struct timeval *, struct rusage *);

void save_times(struct timeval *, struct timeval *);

int main(int argc, char **argv) {
    if (argc != 5) {
        FAILURE_EXIT(1, "Wrong number of arguments\n");
    }
    if (atexit(&free_all) != 0) {
        FAILURE_EXIT(1, "Setting atexit function failed\n");
    }
    load_image(argv[2]);
    load_filter(argv[3]);

    threads_num = 0;
    threads_num = atoi(argv[1]);
    if (threads_num == 0) {
        FAILURE_EXIT(1, "Wrong number of threads\n");
    }

    struct timeval u_start, u_end, s_start, s_end, u_res, s_res;
    struct rusage usage;

    pthread_t *threads = malloc(threads_num * sizeof(pthread_t));
    take_measurements(&u_start, &s_start, &usage);
    for (int i = 0; i < threads_num; i++) {
        int *arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(&threads[i], NULL, filter_image, arg);
    }
    for (int i = 0; i < threads_num; i++) {
        //pthread_join(threads[i], NULL);
    }
    take_measurements(&u_end, &s_end, &usage);
    free(threads);
    timersub(&u_end, &u_start, &u_res);
    timersub(&s_end, &s_start, &s_res);
    save_result(argv[4]);
    save_times(&u_res, &s_res);
}

void free_all() {
    free(image);
    free(filter);
    free(result);
}

void load_image(char *filename) {
    FILE *in = fopen(filename, "r");
    if (in == NULL) {
        FAILURE_EXIT(1, "Input file opening failed\n");
    }
    char line[MAX_LINE_LEN];
    fgets(line, MAX_LINE_LEN, in);
    fgets(line, MAX_LINE_LEN, in);
    width = atoi(strtok(line, " \n"));
    height = atoi(strtok(NULL, " \n"));
    fgets(line, MAX_LINE_LEN, in);
    max_pix_val = atoi(line);
    image = malloc(height * sizeof(int *));
    result = malloc(height * sizeof(int *));
    for (int i = 0; i < height; i++) {
        image[i] = malloc(width * sizeof(int));
        fgets(line, MAX_LINE_LEN, in);
        for (int j = 0; j < width; j++) {
            image[i][j] = (int) strtol(line, NULL, 10);
            if (image[i][j] < 0 || image[i][j] > max_pix_val) {
                FAILURE_EXIT(1, "Wrong pixel value\n");
            }
        }
    }
    fclose(in);
}

void load_filter(char *filename) {
    FILE *fil = fopen(filename, "r");
    if (fil == NULL) {
        FAILURE_EXIT(1, "Filter file opening failed\n");
    }
    char line[MAX_LINE_LEN];
    fgets(line, MAX_LINE_LEN, fil);
    filter_size = atoi(line);
    filter = malloc(filter_size * sizeof(double *));
    for (int i = 0; i < filter_size; i++) {
        filter[i] = malloc(filter_size * sizeof(double));
        fgets(line, MAX_LINE_LEN, fil);
        for (int j = 0; j < filter_size; j++) {
            filter[i][j] = (int) strtol(line, NULL, 10);
        }
    }
    fclose(fil);
}

void save_result(char *filename) {
    FILE *out = fopen(filename, "w");
    if (out == NULL) {
        FAILURE_EXIT(1, "Output file creating failed\n");
    }
    fprintf(out,"P2\n");
    fprintf(out,"%d %d\n",width,height);
    fprintf(out,"%d\n",max_pix_val);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fprintf(out, "%-4d", result[i][j]);
        }
        fprintf(out, "\n");
    }
    fclose(out);
}

double calculate_pixel(int x, int y) {
    double res = 0;
    for (int i = 0; i < filter_size; i++) {
        for (int j = 0; j < filter_size; j++) {
            res += filter[i][j] *
                   image[(int) fmax(1, x - ceil(filter_size / 2) + i)][(int) fmax(1, y - ceil(filter_size / 2) + j)];
        }
    }
    return res;
}

void *filter_image(void *thread_no) {
    int lines = (int) ceil(height / threads_num);
    int from = *(int *) thread_no * lines;
    int to = from + lines;
    if (to > height) {
        to = height;
    }
    for (int i = from; i < to; i++) {
        result[i] = malloc(width * sizeof(int));
    }
    for (int i = from; i < to; i++) {
        for (int j = 0; j < width; j++) {
            result[j][i] = (int) round(calculate_pixel(j, i));
        }
    }
}

void take_measurements(struct timeval *user_time, struct timeval *sys_time, struct rusage *usage) {
    getrusage(RUSAGE_SELF, usage);
    *sys_time = usage->ru_stime;
    *user_time = usage->ru_utime;
}

void save_times(struct timeval *user, struct timeval *system) {
    FILE *times = fopen("times.txt", "a");
    if (times == NULL) {
        printf("Opening times file failed, measurements will not be saved\n");
    }
    fprintf(times, "Image size: %dx%d\n", width, height);
    fprintf(times, "Filter size: %dx%d\n", filter_size, filter_size);
    fprintf(times, "Threads: %d\n", threads_num);
    fprintf(times, "User time: %ld seconds, %ld microseconds\n", user->tv_sec, user->tv_usec);
    fprintf(times, "System time: %ld seconds, %ld microseconds\n", system->tv_sec, system->tv_usec);
    fprintf(times, "\n\n");
    fclose(times);
}
