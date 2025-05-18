#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "treasureFunctions.h"

User_score* get_user_scores(const char *hunt_name, int *count) {
    chdir("proiect");
    DIR *hunt_dir = opendir(hunt_name);
    if (!hunt_dir) {
        perror("Eroare la deschiderea directorului hunt");
        *count = 0;
        return NULL;
    }

    if (chdir(hunt_name) < 0) {
        perror("Eroare la schimbarea directorului hunt");
        closedir(hunt_dir);
        *count = 0;
        return NULL;
    }

    User_score *useri = NULL;
    int index = 0;

    struct dirent *myfile;
    while ((myfile = readdir(hunt_dir)) != NULL) {
        if (strcmp(myfile->d_name, ".") == 0 || strcmp(myfile->d_name, "..") == 0)
            continue;

        if (strstr(myfile->d_name, "log") != NULL || strstr(myfile->d_name, "Log") != NULL)
            continue;

        Treasure treasure;
        int fd = open(myfile->d_name, O_RDONLY);
        if (fd < 0) continue;

        while (read(fd, &treasure, sizeof(Treasure)) == sizeof(Treasure)) {
            int found = 0;
            for (int i = 0; i < index; i++) {
                if (strcmp(useri[i].text, treasure.text) == 0) {
                    useri[i].scor += treasure.val;
                    found = 1;
                    break;
                }
            }

            if (!found) {
                User_score *temp = realloc(useri, (index + 1) * sizeof(User_score));
                if (!temp) {
                    perror("Eroare realloc");
                    close(fd);
                    closedir(hunt_dir);
                    free(useri);
                    chdir("..");
                    *count = 0;
                    return NULL;
                }
                useri = temp;
                strncpy(useri[index].text, treasure.text, sizeof(useri[index].text) - 1);
                useri[index].text[sizeof(useri[index].text) - 1] = '\0';
                useri[index].scor = treasure.val;
                index++;
            }
        }

        close(fd);
    }

    closedir(hunt_dir);
    chdir("..");
    *count = index;
    return useri;
}

void print_scores(User_score *users, int count, const char *hunt_name) {
    printf("Scoruri pentru hunt: %s\n", hunt_name);
    for (int i = 0; i < count; ++i) {
        printf("%29s: %d\n", users[i].text, users[i].scor);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Utilizare: %s <nume_hunt>\n", argv[0]);
        return 1;
    }

    int user_count = 0;
    User_score *users = get_user_scores(argv[1], &user_count);
    if (users) {
        print_scores(users, user_count, argv[1]);
        free(users);
    }

    return 0;
}
