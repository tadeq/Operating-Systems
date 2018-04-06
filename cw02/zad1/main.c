#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

void check_if_open(FILE *file);

int find_command(const char **commands, char *com);

void take_measurements(struct timeval *user_time, struct timeval *sys_time, struct rusage *usage);

void correct_results(struct timeval *u_start, struct timeval *s_start, struct timeval *u_end, struct timeval *s_end);

void generate(FILE *file, int records, int bytes);

void sort(FILE *file, int records, int bytes, char *option);

void copy(FILE *file1, FILE *file2, int records, int bytes, char *option);

int main(int argc, char **argv) {

    if (argc < 5) {
        printf("Too few arguments\n");
        exit(1);
    }

    const char *commands[3];
    commands[0] = "generate";
    commands[1] = "sort";
    commands[2] = "copy";

    struct timeval u_start, u_end, s_start, s_end;
    struct rusage usage;

    int command = find_command(commands, argv[1]);

    FILE * res = fopen("wyniki.txt","a");

    switch (command) {
        case 0:;
            FILE *file0 = fopen(argv[2], "w+");
            check_if_open(file0);
            generate(file0, atoi(argv[3]), atoi(argv[4]));
            break;
        case 1:
            if (argc < 6) {
                printf("Too few arguments\n");
                exit(1);
            }
            FILE *file1 = NULL;
            if (strcmp(argv[5], "sys") == 0) {
                file1 = open(argv[2], O_RDWR);
            } else if (strcmp(argv[5], "lib") == 0) {
                file1 = fopen(argv[2], "r+");
            } else {
                printf("Wrong parameter\n");
                exit(1);
            }
            check_if_open(file1);
            take_measurements(&u_start, &s_start, &usage);
            sort(file1, atoi(argv[3]), atoi(argv[4]), argv[5]);
            take_measurements(&u_end, &s_end, &usage);
            break;
        case 2:
            if (argc < 7) {
                printf("Too few arguments\n");
                exit(1);
            }
            FILE *from = NULL, *to = NULL;
            if (strcmp(argv[6], "sys") == 0) {
                from = open(argv[2], O_RDONLY);
                to = open(argv[3], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
            } else if (strcmp(argv[6], "lib") == 0) {
                from = fopen(argv[2], "r");
                to = fopen(argv[3], "w");
            } else {
                printf("Wrong parameter\n");
                exit(1);
            }
            check_if_open(from);
            check_if_open(to);
            take_measurements(&u_start, &s_start, &usage);
            copy(from, to, atoi(argv[4]), atoi(argv[5]), argv[6]);
            take_measurements(&u_end, &s_end, &usage);
            break;
        default:
            printf("Wrong parameter\n");
            exit(1);
    }
    correct_results(&u_start, &s_start, &u_end, &s_end);
    switch (command) {
        case 1:
            fprintf(res,"Sorting %d records (%d bytes each) with %s functions took:\n"
                           "User time: %ld seconds %ld microseconds\nSystem time: %ld seconds %ld microseconds\n\n"
                           , atoi(argv[3]), atoi(argv[4]), argv[5], u_end.tv_sec - u_start.tv_sec,
                   u_end.tv_usec - u_start.tv_usec,
                   s_end.tv_sec - s_start.tv_sec, s_end.tv_usec - s_start.tv_usec);
            break;
        case 2:
            fprintf(res,"Copying %d records (%d bytes each) with %s functions took:\n"
                           "User time: %ld seconds %ld microseconds\nSystem time: %ld seconds %ld microseconds\n\n"
                           , atoi(argv[4]), atoi(argv[5]), argv[6], u_end.tv_sec - u_start.tv_sec,
                   u_end.tv_usec - u_start.tv_usec,
                   s_end.tv_sec - s_start.tv_sec, s_end.tv_usec - s_start.tv_usec);
            break;
        default:
            break;
    }
    fclose(res);
}

void check_if_open(FILE *file) {
    if (file == NULL) {
        printf("File opening failed\n");
        exit(1);
    }
}

int find_command(const char **commands, char *com) {
    for (int i = 0; i < 5; i++) {
        if (strcmp(com, commands[i]) == 0) return i;
    }
    return -1;
}

void take_measurements(struct timeval *user_time, struct timeval *sys_time, struct rusage *usage) {
    getrusage(RUSAGE_SELF, usage);
    *sys_time = usage->ru_stime;
    *user_time = usage->ru_utime;
}

void correct_results(struct timeval *u_start, struct timeval *s_start, struct timeval *u_end, struct timeval *s_end) {
    if (u_end->tv_usec < u_start->tv_usec && u_end->tv_sec > u_start->tv_sec) {
        u_end->tv_usec += 1000000;
        u_end->tv_sec -= 1;
    }
    if (s_end->tv_usec < u_end->tv_usec && s_end->tv_sec > s_start->tv_sec) {
        s_end->tv_usec += 1000000;
        s_end->tv_sec -= 1;
    }
}

void generate(FILE *file, int records, int bytes) {
    srand(time(NULL));
    char *buf = malloc(records * bytes * sizeof(char));
    for (int i = 0; i < records * bytes; i++) {
        char r = rand() % 26 + 65;
        buf[i] = r;
    }
    for (int i = bytes - 1; i < records * bytes; i += bytes) buf[i] = '\n';
    buf[records * bytes] = '\0';
    fprintf(file, "%s", buf);
    free(buf);
}

void sort(FILE *file, int records, int bytes, char *option) {
    char buf1[bytes], buf2[bytes];
    int sorted = 0;
    int i = 0;
    if (strcmp(option, "lib") == 0) {
        fseek(file, 0, 0);
        while (sorted + 1 < records) {
            fseek(file, bytes * sorted, 0);
            fread(buf1, sizeof(char), bytes, file);
            fread(buf2, sizeof(char), bytes, file);
            i = sorted;
            fseek(file, -bytes, 1);
            while (buf2[0] < buf1[0] && i >= 0) {
                fwrite(buf1, sizeof(char), bytes, file);
                if (fseek(file, -2 * bytes, 1) != 0) break;
                fwrite(buf2, sizeof(char), bytes, file);
                if (fseek(file, -2 * bytes, 1) != 0) break;
                fread(buf1, sizeof(char), bytes, file);
                i--;
            }
            sorted++;
        }
    } else if (strcmp(option, "sys") == 0) {

        lseek(file, 0, SEEK_SET);
        while (sorted + 1 < records) {
            lseek(file, bytes * sorted, SEEK_SET);
            read(file, buf1, bytes * sizeof(char));
            read(file, buf2, bytes * sizeof(char));
            i = sorted;
            lseek(file, -bytes, SEEK_CUR);
            while (buf2[0] < buf1[0] && i >= 0) {
                write(file, buf1, bytes * sizeof(char));
                if (lseek(file, -2 * bytes, SEEK_CUR) < 0) break;
                write(file, buf2, bytes * sizeof(char));
                if (lseek(file, -2 * bytes, SEEK_CUR) < 0) break;
                read(file, buf1, bytes * sizeof(char));
                i--;
            }
            sorted++;
        }
    }
}

void copy(FILE *file1, FILE *file2, int records, int bytes, char *option) {
    char buf[bytes];
    int i = 0;
    if (strcmp(option, "sys") == 0) {
        while (read(file1, buf, bytes * sizeof(char)) > 0 && i < records) {
            write(file2, buf, bytes * sizeof(char));
            i++;
        }
    } else if (strcmp(option, "lib") == 0) {
        while (fread(buf, sizeof(char), bytes, file1) > 0 && i < records) {
            fwrite(buf, sizeof(char), bytes, file2);
            i++;
        }
    }

}