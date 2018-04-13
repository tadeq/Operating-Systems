#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void print_permissions(struct stat *stat);

int compare_dates(char *date1, char *date2);

void traverse(DIR *dir, char *path, struct dirent *dirent, int comp, char *date);

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Too few arguments");
        exit(1);
    }
    char path[2000] = "";
    strcpy(path, argv[1]);
    int comp = 0;
    if (strcmp(argv[2], "<") == 0) comp = -1;
    else if (strcmp(argv[2], ">") == 0) comp = 1;
    else if (strcmp(argv[2], "=") == 0) comp = 0;
    else {
        printf("Wrong argument");
        exit(1);
    }
    DIR *dir = opendir(path);
    if (dir == NULL) {
        printf("No such file or directory");
        exit(1);
    }
    if (argc == 4) {
        struct dirent *dirent;
        traverse(dir, path, dirent, comp, argv[3]);
    } else {
        printf("Wrong argument");
        exit(1);
    }
}

void print_permissions(struct stat *stat) {
    printf((stat->st_mode & S_IRUSR) ? "r" : "-");
    printf((stat->st_mode & S_IWUSR) ? "w" : "-");
    printf((stat->st_mode & S_IXUSR) ? "x" : "-");
    printf((stat->st_mode & S_IRGRP) ? "r" : "-");
    printf((stat->st_mode & S_IWGRP) ? "w" : "-");
    printf((stat->st_mode & S_IXGRP) ? "x" : "-");
    printf((stat->st_mode & S_IROTH) ? "r" : "-");
    printf((stat->st_mode & S_IWOTH) ? "w" : "-");
    printf((stat->st_mode & S_IXOTH) ? "x  " : "-  ");
}

int compare_dates(char *date1, char *date2) {
    char time1[5], time2[5];
    strncpy(time1, date1, 4);
    strncpy(time2, date2, 4);
    time1[4] = '\0';
    time2[4] = '\0';
    int y1, y2, m1, m2, d1, d2;
    y1 = atoi(time1);
    y2 = atoi(time2);
    strncpy(time1, &date1[5], 2);
    strncpy(time2, &date2[5], 2);
    time1[2] = '\0';
    time2[2] = '\0';
    m1 = atoi(time1);
    m2 = atoi(time2);
    strncpy(time1, &date1[8], 2);
    strncpy(time2, &date2[8], 2);
    time1[2] = '\0';
    time2[2] = '\0';
    d1 = atoi(time1);
    d2 = atoi(time2);
    if (y1 == 0 || y2 == 0 || m1 == 0 || m2 == 0 || d1 == 0 || d2 == 0) {
        printf("Wrong date format");
        exit(1);
    }
    if (y1 > y2) return 1;
    else if (y1 < y2) return -1;
    else if (m1 > m2) return 1;
    else if (m1 < m2) return -1;
    else if (d1 > d2) return 1;
    else if (d1 < d2) return -1;
    else return 0;
}

void traverse(DIR *dir, char *path, struct dirent *dirent, int comp, char *date) {
    char curr[2000] = "";
    char buf[20] = "";
    struct tm *tm;
    struct stat file_info;
    dir = opendir(path);
    if (dir == NULL) {
        return;
    }
    dirent = readdir(dir);
    while (dirent != NULL) {
        strncpy(curr, path, strlen(path));
        strcat(curr, "/");
        strcat(curr, dirent->d_name);
        if (lstat(curr, &file_info) == 0) {
            if (strcmp(dirent->d_name, ".") != 0 && strcmp(dirent->d_name, "..") != 0) {
                tm = localtime(&file_info.st_mtime);
                strftime(buf, 20, "%Y-%m-%d %H:%M:%S", tm);
                if (S_ISREG(file_info.st_mode) && comp == compare_dates(buf, date)) {
                    print_permissions(&file_info);
                    printf("%-12d%s  %s\n", file_info.st_size, buf, curr);
                } else if (S_ISDIR(file_info.st_mode)) {
                    pid_t child_pid;
                    if (child_pid = fork() == 0) {
                        traverse(dir, curr, dirent, comp, date);
                        exit(0);
                    }
                }
            }
        } else return;
        strncpy(curr, path, strlen(curr));
        dirent = readdir(dir);
    }
    closedir(dir);
}