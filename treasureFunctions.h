#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

typedef enum {
    ADD,
    LIST,
    VIEW,
    REMOVE_TREASURE,
    REMOVE_HUNT
} Options;

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

void calculateScore();
void createLog(const char *hunt_id, const char *mes);
void list(const char *hunt_id);
void add(const char *hunt_id, Treasure treasure);
void list_hunts();
void view(const char *treasure_id);
void remove_treasure(const char *treasure_id);
void remove_hunt(const char *hunt_id);
Treasure treasure_generator(const char *nume);