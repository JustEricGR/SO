#include "treasureFunctions.h"

#define CHUNK 10


void createLog(const char *hunt_id, const char *mes) {
    char logName[256];
    snprintf(logName, sizeof(logName), "Log%s.txt", hunt_id);

    int new_file = access(logName, F_OK) != 0;

    int fd = open(logName, O_WRONLY | O_CREAT | O_APPEND, 0644);
    
    if (new_file) {
        char symlinkName[300];
        snprintf(symlinkName, sizeof(symlinkName), "../logged_hunt-%s", hunt_id);
        symlink(logName, symlinkName);
    }

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
    char huntTreasure[200]="";
    sprintf(huntTreasure,"Treasures%s",hunt_id);
    //struct stat mystat;

    mydir = opendir("proiect");
    chdir("proiect");
    myfile = readdir(mydir);
    if (mydir == NULL) {
        perror("Eroare la deschiderea directorului proiect");
        return;
    }
    char flag=0;
    

    while((myfile = readdir(mydir)) != NULL)
    {
        if (strcmp(myfile->d_name, ".") == 0 || strcmp(myfile->d_name, "..") == 0)
        continue;
        // stat(myfile->d_name, &mystat);    
        //printf("%d",mystat.st_size);
        //printf(" %s\n", myfile->d_name);
        if(strcmp(myfile->d_name, hunt_id) == 0) {
            flag = 1;
            chdir(hunt_id);
            
            break;
        }
        
    }
    
    
    
    //int fd = open(huntTreasure, O_RDWR | O_CREAT | O_APPEND, 0644);
    if(flag) {
        //struct dirent *treasureFile;
        
        int fd = open(huntTreasure, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd == -1) {
            perror("Eroare la open() pentru treasures.dat");
        } else {
            printf("Scriu in %s\n", huntTreasure);
            ssize_t w = write(fd, &treasure, sizeof(Treasure));
            if (w == -1)
                perror("Eroare la write()");
            else
                printf("Am scris %ld bytes in %s\n", w, huntTreasure);
                        
            close(fd);
        }
        

        char aux[300] = "";
        sprintf(aux, "Added %s to %s\n", treasure.id, hunt_id);
        strcat(logMessage, aux);
        createLog(hunt_id, logMessage);
        free(logMessage);
        chdir("..");
    }
    

    else {
        int result = mkdir(hunt_id, 0777);
        chdir(hunt_id);
        
        int fd = open(huntTreasure, O_RDWR | O_CREAT | O_APPEND, 0664);
        if (fd == -1) {
            perror("Eroare la open() pentru treasures.dat");
        } else {
            printf("Scriu in %s\n", huntTreasure);
            write(fd, &treasure, sizeof(Treasure));
            close(fd);
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
    //mydir = opendir("proiect");
}

void list(const char *hunt_id) {
    char *logMessage=(char*)malloc(500*sizeof(char));
    logMessage[0]=0;
    DIR *mydir;
    
    struct dirent *myfile;
    //struct stat mystat;

    mydir = opendir("proiect");
    chdir("proiect");
    //myfile = readdir(mydir);
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
                if (strcmp(myfile->d_name, ".") == 0 || strcmp(myfile->d_name, "..") == 0 || strstr(myfile->d_name, "Log")!=NULL)
                    continue;
                else {
                    Treasure treasure;
                    int fd = open(myfile->d_name, O_RDONLY);
                    while(read(fd, &treasure, sizeof(Treasure)) == sizeof(Treasure)) {
                        printf("%15s %29s %f %f %29s %d\n",treasure.id, treasure.text, treasure.coordinates.latitudine, treasure.coordinates.longitudine, treasure.clue, treasure.val);
                    }
                    close(fd);
                }
                
                //printf("Nume fisier: %s\n",myfile->d_name);
                //read(fd, &treasure, sizeof(Treasure));
                
                
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

void list_hunts() {
    DIR *mydir = opendir("proiect");
    if (!mydir) {
        perror("Eroare la deschiderea directorului 'proiect'");
        return;
    }

    if (chdir("proiect") < 0) {
        perror("Eroare la schimbarea directorului");
        closedir(mydir);
        return;
    }

    struct dirent *myfile;
    char flag = 0;

    while ((myfile = readdir(mydir)) != NULL) {
        if (strcmp(myfile->d_name, ".") == 0 || strcmp(myfile->d_name, "..") == 0)
            continue;

        struct stat st;
        if (stat(myfile->d_name, &st) < 0 || !S_ISDIR(st.st_mode))
            continue;

        if (strstr(myfile->d_name, "log") != NULL || strstr(myfile->d_name, "Log") != NULL)
            continue;

        DIR *hunt_dir = opendir(myfile->d_name);
        if (!hunt_dir)
            continue;

        if (chdir(myfile->d_name) < 0) {
            closedir(hunt_dir);
            continue;
        }

        struct dirent *myfile1;
        while ((myfile1 = readdir(hunt_dir)) != NULL) {
            if (strcmp(myfile1->d_name, ".") == 0 || strcmp(myfile1->d_name, "..") == 0)
                continue;

            if (strstr(myfile1->d_name, "Log") != NULL || strstr(myfile1->d_name, "log") != NULL)
                continue;

            Treasure treasure;
            int fd = open(myfile1->d_name, O_RDONLY);
            if (fd < 0) continue;

            while (read(fd, &treasure, sizeof(Treasure)) == sizeof(Treasure)) {
                printf("%15s %29s %f %f %29s %d\n",
                       treasure.id, treasure.text,
                       treasure.coordinates.latitudine,
                       treasure.coordinates.longitudine,
                       treasure.clue, treasure.val);
                flag = 1;
            }

            close(fd);
        }

        closedir(hunt_dir);
        chdir("..");
    }

    closedir(mydir);

    if (!flag)
        printf("Nu exista huntul\n");
}


void view(const char *treasure_id) {
    DIR *main_dir;
    struct dirent *hunt_entry;
    char *logMessage=(char*)malloc(500*sizeof(char));
    logMessage[0]=0;
    char found = 0;
    main_dir = opendir("proiect");

    chdir("proiect");

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
            if (strcmp(file_entry->d_name, ".") == 0 || strcmp(file_entry->d_name, "..") == 0 || strstr(file_entry->d_name, "Log")!=NULL)
                continue;

            else {
                
                Treasure treasure;
                int fd = open(file_entry->d_name, O_RDONLY);
                if (fd < 0) {
                    perror("Eroare la deschiderea fisierului");
                    break;
                }

                while(read(fd, &treasure, sizeof(Treasure)) == sizeof(Treasure)) {
                    if(!strcmp(treasure.id, treasure_id)) {
                        found=1;
                        printf("Nume fisier: %s\n", file_entry->d_name);
                        printf("%9s %29s %f %f %29s %d\n", treasure.id, treasure.text,
                       treasure.coordinates.latitudine, treasure.coordinates.longitudine,
                       treasure.clue, treasure.val);
                       break;
                    }
                }
                close(fd);

                
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
    char *logMessage = (char *)malloc(500 * sizeof(char));
    logMessage[0] = 0;
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
            if (strcmp(file_entry->d_name, ".") == 0 || strcmp(file_entry->d_name, "..") == 0 || strstr(file_entry->d_name, "Log") != NULL)
                continue;

            int fd = open(file_entry->d_name, O_RDWR);
            if (fd < 0) {
                perror("Eroare la deschiderea fisierului");
                continue;
            }

            Treasure *all = NULL;
            int size = 0, capacity = CHUNK;
            all = malloc(capacity * sizeof(Treasure));
            Treasure t;

            while (read(fd, &t, sizeof(Treasure)) == sizeof(Treasure)) {
                if (strcmp(t.id, treasure_id) != 0) {
                    if (size >= capacity) {
                        capacity += CHUNK;
                        all = realloc(all, capacity * sizeof(Treasure));
                    }
                    all[size++] = t;
                } 
                else {
                    found = 1;
                }
            }

            if (found) {
                lseek(fd, 0, SEEK_SET);
                write(fd, all, size * sizeof(Treasure));
                ftruncate(fd, size * sizeof(Treasure));

                char aux[300] = "";
                sprintf(aux, "Removed %s from %s\n", treasure_id, hunt_entry->d_name);
                strcat(logMessage, aux);
                createLog(hunt_entry->d_name, logMessage);
                free(logMessage);
                free(all);
                close(fd);
                closedir(hunt_dir);
                chdir("..");
                closedir(main_dir);
                return;
            }

            close(fd);
        }

        closedir(hunt_dir);
        chdir("..");
    }

    closedir(main_dir);

    if (!found)
        printf("Comoara cu ID-ul '%s' nu a fost gasita.\n", treasure_id);

    free(logMessage);
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

    
    int clue_index = rand() % (sizeof(clues) / sizeof(clues[0]));
    strncpy(t.clue, clues[clue_index], sizeof(t.clue));

    
    t.val = rand() % 100 + 1;

    return t;
}

