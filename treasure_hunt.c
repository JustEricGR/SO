#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include<string.h>

typedef struct {
    float latitudine;
    float longitudine;
} GPS;

typedef struct {
    int id;
    char text[30];
    GPS coordinates;
    char clue[30];
    int val;
} Treasure;


typedef struct {
    int hunt_id;
    Treasure *trasures;
} Hunt;

void add(int hunt_id, Treasure treasure) {
    char aux[10]="";
    sprintf(aux,"%d",hunt_id);

    DIR *mydir;
    struct dirent *myfile;
    struct stat mystat;

    mydir = opendir(argv[1]);
    while((myfile = readdir(mydir)) != NULL)
    {
        stat(myfile->d_name, &mystat);    
        printf("%d",mystat.st_size);
        printf(" %s\n", myfile->d_name);
    }
    closedir(mydir);


    int result = mkdir(aux, S_IRWXU | S_IRWXG | S_IRWXO);

    if(result!=0) {
        printf("Eroare la creare fisier\n");
    }
    else {
        printf("Succes\n");
    }


}

int main(void) {
    add(2345);
}

