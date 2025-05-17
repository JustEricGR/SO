// #include "treasure_hunt.c"
// #include <sys/wait.h>
// #include <signal.h>
// #include <unistd.h>
// #include <stdlib.h>
// #include <stdio.h>

// #define CHUNK 10

// int monitor_pipe[2];

// typedef enum {
//     LIST_HUNTS,
//     LIST_TREASURES,
//     VIEW_TREASURE,
//     CALCULATE_SCORE,
//     STOP_MONITOR,
//     EXIT
// } Options1;

// int monitor_active = 0;
// int monitor_stopping = 0;
// pid_t monitor_pid;

// // Handler pentru terminarea procesului copil (monitor)
// void handle_sigchld(int sig) {
//     int status;
//     wait(&status);
//     monitor_active = 0;
//     monitor_stopping = 0;
//     printf("\nMonitor terminated with status %d\n", WEXITSTATUS(status));
// }

// // Handler pentru SIGUSR2 (cerere de oprire monitor)
// void handle_sigusr2(int sig) {
//     printf("[Monitor] Primit semnal de oprire (SIGUSR2)...\n");
//     usleep(1000000); 
//     exit(0);
// }

// void start_monitor() {
//     if (pipe(monitor_pipe) == -1) {
//         perror("Eroare la crearea pipe-ului");
//         exit(EXIT_FAILURE);
//     }

//     pid_t pid = fork();
//     if (pid < 0) {
//         perror("Eroare la fork");
//         exit(EXIT_FAILURE);
//     }

//     if (pid == 0) {
//         // Proces monitor
//         struct sigaction sa_usr;
//         sa_usr.sa_handler = handle_sigusr2;
//         sigemptyset(&sa_usr.sa_mask);
//         sa_usr.sa_flags = 0;
//         sigaction(SIGUSR2, &sa_usr, NULL);

//         close(monitor_pipe[0]); 

//         printf("[Monitor] Ruleaza in fundal (PID = %d)\n", getpid());

//         while (1) pause(); // asteapta comenzi prin semnale
//     } else {
//         // Proces principal
//         monitor_pid = pid;
//         monitor_active = 1;
//         close(monitor_pipe[1]); // procesul principal citeste
//     }
// }



// int main(int argc, char **argv) {
//     struct sigaction sa;
//     sa.sa_handler = handle_sigchld;
//     sigemptyset(&sa.sa_mask);
//     sa.sa_flags = 0;
//     sigaction(SIGCHLD, &sa, NULL);

//     start_monitor();

//     Options1 op;
//     char flag = 0;

//     while (1) {
//         printf("Dati o optiune: ");
//         scanf("%u", &op);
//         switch (op) {
//             case LIST_HUNTS:
//                 list_hunts();
//                 break;
//             case LIST_TREASURES: {
//                 char aux[15];
//                 printf("Dati numele unui hunt: ");
//                 scanf("%14s", aux);
//                 list(aux);
//                 break;
//             }
//             case VIEW_TREASURE: {
//                 char aux[15];
//                 printf("Dati numele unui treasure: ");
//                 scanf("%14s", aux);
//                 view(aux);
//                 break;
//             }
//             case CALCULATE_SCORE:
//                 calculateScore();  
//                 if (monitor_active) {
//                     kill(monitor_pid, SIGUSR2);
//                     monitor_stopping = 1;
//                 }
//                 break;
//             case STOP_MONITOR:
//                 if (monitor_active) {
//                     kill(monitor_pid, SIGUSR2);
//                     monitor_stopping = 1;
//                 } else {
//                     printf("Monitor deja oprit.\n");
//                 }
//                 break;
//             case EXIT:
//                 if (monitor_active) {
//                     printf("Monitorul ruleaza. Sa fie oprit inainte (STOP_MONITOR).\n");
//                 } else {
//                     flag = 1;
//                 }
//                 break;
//         }

//         if (flag) break;
//     }

//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include "treasure_hunt.c"

int monitor_pipe[2]; // pipe[0] = read (main), pipe[1] = write (monitor)

typedef enum {
    LIST_HUNTS,
    LIST_TREASURES,
    VIEW_TREASURE,
    CALCULATE_SCORE,
    STOP_MONITOR,
    EXIT
} Options1;

int monitor_active = 0;
pid_t monitor_pid;

void handle_sigchld(int sig) {
    int status;
    wait(&status);
    monitor_active = 0;
    printf("\nMonitor terminated with status %d\n", WEXITSTATUS(status));
}

void handle_sigusr2(int sig) {
    printf("[Monitor] Primit semnal de oprire (SIGUSR2)...\n");
    exit(0);
}

void monitor_loop() {
    close(monitor_pipe[0]); // monitorul scrie
    struct sigaction sa_usr;
    sa_usr.sa_handler = handle_sigusr2;
    sigemptyset(&sa_usr.sa_mask);
    sa_usr.sa_flags = 0;
    sigaction(SIGUSR2, &sa_usr, NULL);

    while (1) {
        // Când primește semnal (simulatează o comandă de la main)
        pause();

        // Când primește SIGUSR1 => execută calculate_score
        DIR *mydir = opendir("proiect");
        if (!mydir) {
            perror("Monitor: Eroare deschidere 'proiect'");
            continue;
        }

        struct dirent *entry;
        while ((entry = readdir(mydir)) != NULL) {
            if (entry->d_type == DT_DIR && strncmp(entry->d_name, "Hunt", 4) == 0) {
                int pfd[2];
                if (pipe(pfd) == -1) continue;

                pid_t pid = fork();
                if (pid == 0) {
                    // copil: redirectare și exec
                    close(pfd[0]);
                    dup2(pfd[1], STDOUT_FILENO);
                    execl("./score_calc", "./score_calc", entry->d_name, NULL);
                    perror("exec score_calc");
                    exit(1);
                } else {
                    // monitor: citește output din pfd[0] și îl scrie în monitor_pipe[1]
                    close(pfd[1]);
                    char buffer[1024];
                    ssize_t bytesRead;
                    while ((bytesRead = read(pfd[0], buffer, sizeof(buffer))) > 0) {
                        write(monitor_pipe[1], buffer, bytesRead);
                    }
                    close(pfd[0]);
                    waitpid(pid, NULL, 0);
                }
            }
        }
        closedir(mydir);

        // trimite semnal de sfârșit de transmisie
        char end = '\0';
        write(monitor_pipe[1], &end, 1);
    }
}

void start_monitor() {
    if (pipe(monitor_pipe) == -1) {
        perror("Eroare la crearea pipe-ului");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("Eroare la fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        monitor_loop(); // intră în loopul monitorului
        exit(0);
    } else {
        monitor_pid = pid;
        monitor_active = 1;
        close(monitor_pipe[1]); // main citește
    }
}

void read_monitor_output() {
    char buffer[1024];
    ssize_t bytesRead;

    printf("[MAIN] Rezultate calculate_score:\n");
    while ((bytesRead = read(monitor_pipe[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        if (buffer[0] == '\0') break; // terminator de final transmisie
        printf("%s", buffer);
    }
    printf("\n[MAIN] Sfârșit rezultate.\n");
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    start_monitor();

    Options1 op;
    int running = 1;

    while (running) {
        printf("\nMeniu:\n");
        printf("0. List hunts\n");
        printf("1. List treasures\n");
        printf("2. View treasure\n");
        printf("3. Calculate score\n");
        printf("4. Stop monitor\n");
        printf("5. Exit\n");
        printf("Optiune: ");
        scanf("%u", &op);

        switch (op) {
            case LIST_HUNTS:
                list_hunts();
                break;
            case LIST_TREASURES:
                char aux[15];
                printf("Dati numele unui hunt: ");
                scanf("%14s", aux);
                list(aux);
                break;
            case VIEW_TREASURE:
                char aux1[15];
                printf("Dati numele unui treasure: ");
                scanf("%14s", aux1);
                view(aux1);
                break;
            case CALCULATE_SCORE:
                calculateScore();
                if (monitor_active) {
                    kill(monitor_pid, SIGUSR1);
                    read_monitor_output();
                } 
                else {
                    printf("Monitorul nu este activ!\n");
                }
                break;
            case STOP_MONITOR:
                if (monitor_active) {
                    kill(monitor_pid, SIGUSR2);
                    waitpid(monitor_pid, NULL, 0);
                    monitor_active = 0;
                } 
                else {
                    printf("Monitor deja oprit.\n");
                }
                break;
            case EXIT:
                if (monitor_active) {
                    printf("Opriti mai intai monitorul (STOP_MONITOR).\n");
                } 
                else {
                    running = 0;
                }
                break;
            default:
                printf("Optiune invalida.\n");
        }
    }

    return 0;
}
