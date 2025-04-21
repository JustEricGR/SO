#include "treasure_hunt.c"

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
