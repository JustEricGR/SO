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
    const char *hunt_id;
    Treasure *trasures;
} Hunt;

void createLog(const char *hunt_id, const char *mes) {
    char logName[256];
    snprintf(logName, sizeof(logName), "Log%s.txt", hunt_id);

    int fd = open(logName, O_WRONLY | O_CREAT | O_APPEND, 0644);
    

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "[%Y-%m-%d %H:%M:%S] ", tm_info);

    write(fd, timeStr, strlen(timeStr));
    write(fd, mes, strlen(mes));
    write(fd, "\n", 1);
    close(fd);
}

void add(const char *hunt_id, Treasure treasure) {
    char *logMessage=(char*)malloc(500*sizeof(char));
    logMessage[0]=0;
    DIR *mydir;
    struct dirent *myfile;
    //struct stat mystat;

    mydir = opendir("proiect");
    chdir("proiect");
    //myfile = readdir(mydir);
    char flag=0;

    while((myfile = readdir(mydir)) != NULL)
    {
        // stat(myfile->d_name, &mystat);    
        // printf("%d",mystat.st_size);
        // printf(" %s\n", myfile->d_name);
        if(strcmp(myfile->d_name, hunt_id) == 0) {
            flag = 1;
            chdir(hunt_id);
            int fd = open(treasure.id, O_RDWR | O_CREAT | O_TRUNC, 0644);
            
            write(fd, &treasure, sizeof(Treasure));
            close(fd);
            
            char aux[300] = "";
            sprintf(aux, "Added %s to %s\n", treasure.id, hunt_id);
            strcat(logMessage, aux);
            createLog(hunt_id, logMessage);
            free(logMessage);
            chdir("..");
            break;
        }
        
    }
    

    if(!flag) {
        int result = mkdir(hunt_id, S_IRWXU | S_IRWXG | S_IRWXO);
        chdir(hunt_id);
        
        int fd = open(treasure.id, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

        write(fd, &treasure, sizeof(Treasure));
        close(fd);
        struct stat mystat;
        if (stat(hunt_id, &mystat) == 0) {
            printf("Size: %ld\n", mystat.st_size);
            printf("Ultima modificare: %s", ctime(&mystat.st_mtime));
            strcat(logMessage, ctime(&mystat.st_mtime));
        }

        char aux[300] = "";
        sprintf(aux, "Added %s to %s\n", treasure.id, hunt_id);
        strcat(logMessage, aux);
        createLog(hunt_id, logMessage);
        free(logMessage);
        chdir("..");

        if(result!=0) {
        printf("Eroare la creare fisier\n");
        }
        else {
            printf("Succes\n");
        }
    }   

    

    closedir(mydir);
    mydir = opendir("proiect");
}

void list(const char *hunt_id) {
    char *logMessage=(char*)malloc(500*sizeof(char));
    logMessage[0]=0;
    DIR *mydir;
    struct dirent *myfile;
    //struct stat mystat;

    mydir = opendir("proiect");
    chdir("proiect");
    myfile = readdir(mydir);
    char flag=0;
    while((myfile=readdir(mydir)) != NULL) {
        if(!strcmp(myfile->d_name, hunt_id)) {
            flag=1;
            //mydir = opendir(hunt_id);
            //printf("Nume: %s\n",hunt_id);
            struct stat mystat;
            if (stat(hunt_id, &mystat) == 0) {
                printf("Size: %ld\n", mystat.st_size);
                printf("Ultima modificare: %s", ctime(&mystat.st_mtime));
            }
            DIR *hunt_dir = opendir(hunt_id);
            chdir(hunt_id);
            while((myfile=readdir(hunt_dir)) != NULL) {
                if (strcmp(myfile->d_name, ".") == 0 || strcmp(myfile->d_name, "..") == 0)
                    continue;
                Treasure treasure;
                int fd = open(myfile->d_name, O_RDONLY);
                //printf("Nume fisier: %s\n",myfile->d_name);
                read(fd, &treasure, sizeof(Treasure));
                printf("%9s %s %f %f %29s %d\n",treasure.id, treasure.text, treasure.coordinates.latitudine, treasure.coordinates.longitudine, treasure.clue, treasure.val);
                close(fd);
            }
            char aux[300] = "";
            sprintf(aux, "Listing treasures in hunt %s\n", hunt_id);
            strcat(logMessage, aux);
            createLog(hunt_id, logMessage);
            free(logMessage);
            closedir(hunt_dir);
            chdir("..");
            break;
        }
    }

    if(!flag) printf("Nu exista huntul\n");
}

void view(const char *treasure_id) {
    DIR *main_dir;
    struct dirent *hunt_entry;
    char *logMessage=(char*)malloc(500*sizeof(char));
    logMessage[0]=0;

    main_dir = opendir("proiect");

    chdir("proiect");

    char found = 0;

    while ((hunt_entry = readdir(main_dir)) != NULL) {
        if (strcmp(hunt_entry->d_name, ".") == 0 || strcmp(hunt_entry->d_name, "..") == 0)
            continue;

        struct stat st;
        if (stat(hunt_entry->d_name, &st) != 0 || !S_ISDIR(st.st_mode))
            continue;

        DIR *hunt_dir = opendir(hunt_entry->d_name);
        if (!hunt_dir) continue;

        chdir(hunt_entry->d_name);

        struct dirent *file_entry;
        while ((file_entry = readdir(hunt_dir)) != NULL) {
            if (strcmp(file_entry->d_name, ".") == 0 || strcmp(file_entry->d_name, "..") == 0)
                continue;

            if (!strcmp(file_entry->d_name, treasure_id)) {
                found = 1;
                Treasure treasure;
                int fd = open(file_entry->d_name, O_RDONLY);
                if (fd < 0) {
                    perror("Eroare la deschiderea fisierului");
                    break;
                }

                read(fd, &treasure, sizeof(Treasure));
                close(fd);

                printf("Nume fisier: %s\n", file_entry->d_name);
                printf("%9s %29s %f %f %29s %d\n", treasure.id, treasure.text,
                       treasure.coordinates.latitudine, treasure.coordinates.longitudine,
                       treasure.clue, treasure.val);
                char aux[300] = "";
                sprintf(aux, "Viewed %s from %s\n", treasure.id, hunt_entry->d_name);
                strcat(logMessage, aux);
                createLog(hunt_entry->d_name, logMessage);
                free(logMessage);
                break;
            }
        }

        closedir(hunt_dir);
        chdir("..");

        if (found)
            break;
    }

    closedir(main_dir);

    if (!found)
        printf("Comoara cu ID-ul '%s' nu a fost gasita.\n", treasure_id);
}

void remove_treasure(const char *treasure_id) {
    DIR *main_dir;
    struct dirent *hunt_entry;
    char *logMessage=(char*)malloc(500*sizeof(char));
    logMessage[0]=0;
    main_dir = opendir("proiect");

    chdir("proiect");

    char found = 0;

    while ((hunt_entry = readdir(main_dir)) != NULL) {
        if (strcmp(hunt_entry->d_name, ".") == 0 || strcmp(hunt_entry->d_name, "..") == 0)
            continue;

        struct stat st;
        if (stat(hunt_entry->d_name, &st) != 0 || !S_ISDIR(st.st_mode))
            continue;

        DIR *hunt_dir = opendir(hunt_entry->d_name);
        if (!hunt_dir) continue;

        chdir(hunt_entry->d_name);

        struct dirent *file_entry;
        while ((file_entry = readdir(hunt_dir)) != NULL) {
            if (strcmp(file_entry->d_name, ".") == 0 || strcmp(file_entry->d_name, "..") == 0)
                continue;

            if (!strcmp(file_entry->d_name, treasure_id)) {
                found = 1;
                remove(file_entry->d_name);
                char aux[300] = "";
                sprintf(aux, "Removed %s from %s\n", treasure_id, hunt_entry->d_name);
                strcat(logMessage, aux);
                createLog(hunt_entry->d_name, logMessage);
                free(logMessage);
                break;
            }
        }

        closedir(hunt_dir);
        chdir("..");

        if (found)
            break;
    }

    closedir(main_dir);
    //chdir(".."); 

    if (!found)
        printf("Comoara cu ID-ul '%s' nu a fost gasita.\n", treasure_id);

}


void remove_hunt(const char *hunt_id) {
    DIR *main_dir;
    struct dirent *hunt_entry;

    main_dir = opendir("proiect");

    chdir("proiect");

    char found = 0;

    while ((hunt_entry = readdir(main_dir)) != NULL) {
        if (strcmp(hunt_entry->d_name, ".") == 0 || strcmp(hunt_entry->d_name, "..") == 0)
            continue;

        struct stat st;
        if (stat(hunt_entry->d_name, &st) != 0 || !S_ISDIR(st.st_mode))
            continue;

        

        struct dirent *file_entry;
        if(!strcmp(hunt_entry->d_name,hunt_id)) {
            DIR *hunt_dir = opendir(hunt_entry->d_name);
            chdir(hunt_entry->d_name);
            while ((file_entry = readdir(hunt_dir)) != NULL) {
                remove(file_entry->d_name);
            }
            closedir(hunt_dir);
            chdir("..");
            rmdir(hunt_id);
            
        }
        
        if (found)
            break;
    }

    closedir(main_dir);
    chdir(".."); 

    
}


Treasure treasure_generator(const char *nume) {
    srand(time(NULL));
    const char *texts[] = {
    "Comoara stralucitoare",
    "Inelul fermecat",
    "Sabia uitata",
    "Cartea magica",
    "Elixirul vietii",
    "Cufarul de aur",
    "Medalionul regal",
    "Scroll-ul antic",
    "Sfera misterioasa",
    "Lanterna eterna"
};

const char *clues[] = {
    "langa copacul batran",
    "sub piatra mare",
    "in spatele cascadei",
    "la umbra turnului",
    "in pantecul dragonului",
    "pe varful muntelui",
    "in pestera ascunsa",
    "sub nisipuri miscatoare",
    "in inima padurii",
    "in ruinele templelor"
};

    Treasure t;
    strncpy(t.id, nume, sizeof(t.id));
    t.id[sizeof(t.id) - 1] = '\0';

    int text_index = rand() % (sizeof(texts) / sizeof(texts[0]));
    strncpy(t.text, texts[text_index], sizeof(t.text));

    t.coordinates.latitudine = ((float)(rand() % 18000) / 100.0f) - 90.0f;
    t.coordinates.longitudine = ((float)(rand() % 36000) / 100.0f) - 180.0f;

    // Alege indiciu aleatoriu
    int clue_index = rand() % (sizeof(clues) / sizeof(clues[0]));
    strncpy(t.clue, clues[clue_index], sizeof(t.clue));

    
    t.val = rand() % 100 + 1;

    return t;
}

int main(int argc, char **argv) {
    Options opt;
    if(!strcmp(argv[1],"--add")) opt=ADD;
    else if(!strcmp(argv[1],"--list")) opt=LIST;
    else if(!strcmp(argv[1],"--view")) opt=VIEW;
    else if(!strcmp(argv[1],"--remove_treasure")) opt=REMOVE_TREASURE;
    else if(!strcmp(argv[1],"--remove_hunt")) opt=REMOVE_HUNT;
    switch(opt) {
        case ADD :
            Treasure aux = treasure_generator(argv[3]);
            add(argv[2], aux);
            break;
        case LIST :
            list(argv[2]);
            break;
        case VIEW :
            view(argv[2]);
            break;
        case REMOVE_TREASURE :
            remove_treasure(argv[2]);
            break;
        case REMOVE_HUNT :
            remove_hunt(argv[2]);
            break;
    }
    
}
