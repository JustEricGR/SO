#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

typedef struct {
    float latitudine;
    float longitudine;
} GPS;

typedef struct {
    char id[15];
    char text[30];
    GPS coordinates;
    char clue[30];
    int val;
} Treasure;

typedef struct {
    char text[30];
    int scor;
} User_score;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_dir>\n", argv[0]);
        return 1;
    }

    char path[256];
    snprintf(path, sizeof(path), "proiect/%s", argv[1]);

    DIR *d = opendir(path);
    if (!d) {
        perror("Deschidere hunt_dir");
        return 1;
    }

    User users[100];
    int user_count = 0;

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_type != DT_REG) continue;

        char fpath[512];
        snprintf(fpath, sizeof(fpath), "%s/%s", path, entry->d_name);
        int fd = open(fpath, O_RDONLY);
        if (fd < 0) continue;

        Treasure t;
        while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
            int found = 0;
            for (int i = 0; i < user_count; ++i) {
                if (strcmp(users[i].user, t.text) == 0) {
                    users[i].score += t.val;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                strncpy(users[user_count].user, t.text, sizeof(t.text));
                users[user_count].score = t.val;
                user_count++;
            }
        }

        close(fd);
    }

    closedir(d);

    printf("Rezultate pentru %s:\n", argv[1]);
    for (int i = 0; i < user_count; i++) {
        printf("%25s: %d\n", users[i].user, users[i].score);
    }

    return 0;
}