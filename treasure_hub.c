#include "treasure_hunt.c"
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define CHUNK 10

typedef enum {
    LIST_HUNTS,
    LIST_TREASURES,
    VIEW_TREASURE,
    STOP_MONITOR,
    EXIT
} Options1;

int monitor_active = 0;
int monitor_stopping = 0;
pid_t monitor_pid;

// Handler pentru terminarea procesului copil (monitor)
void handle_sigchld(int sig) {
    int status;
    wait(&status);
    monitor_active = 0;
    monitor_stopping = 0;
    printf("\nMonitor terminated with status %d\n", WEXITSTATUS(status));
}

// Handler pentru SIGUSR2 (cerere de oprire monitor)
void handle_sigusr2(int sig) {
    printf("[Monitor] Primit semnal de oprire (SIGUSR2)...\n");
    usleep(1000000); // Simulează o oprire întârziată
    exit(0);
}

void start_monitor() {
    int pid;
    Options1 op;
    char flag = 0;

    if ((pid = fork()) < 0) {
        perror("Eroare la fork");
        return;
    }

    if (pid > 0) {
    
        monitor_pid = pid;
        monitor_active = 1;

        while (1) {
            printf("Dati o optiune: ");
            scanf("%u", &op);
            switch (op) {
                case LIST_HUNTS:
                    list_hunts();
                    break;
                case LIST_TREASURES: {
                    char aux[15] = "";
                    printf("Dati numele unui hunt: ");
                    scanf("%14s", aux);
                    list(aux);
                    break;
                }
                case VIEW_TREASURE: {
                    char aux[15] = "";
                    printf("Dati numele unui treasure: ");
                    scanf("%14s", aux);
                    view(aux);
                    break;
                }
                case STOP_MONITOR:
                    if (!monitor_active) {
                        printf("Monitor is already stopped.\n");
                    } else {
                        kill(monitor_pid, SIGUSR2);
                        monitor_stopping = 1;
                    }
                    break;
                case EXIT:
                    if (monitor_active) {
                        printf("Monitor still running! Use stop_monitor first.\n");
                    } else {
                        flag = 1;
                        exit(0);
                    }
                    break;
            }
            if (flag) break;
        }

    } else {
    
        struct sigaction sa_usr;
        sa_usr.sa_handler = handle_sigusr2;
        sigemptyset(&sa_usr.sa_mask);
        sa_usr.sa_flags = 0;
        sigaction(SIGUSR2, &sa_usr, NULL);

        printf("[Monitor] Ruleaza in fundal (pid=%d)...\n", getpid());
        while (1) pause();
    }
}

int main(int argc, char **argv) {
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    start_monitor();
}
