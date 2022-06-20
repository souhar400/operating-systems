
#include "lib/OSMPlib.h"
int main(int argc, char** argv){
    int rank = 0, size = 0;
    OSMP_Init(&argc, argv);
    OSMP_Rank(&rank);
    OSMP_Size(&size);

    int root = 1;
    if(rank == 1){
        printf("Sender Prozess %d\n", rank);
        char *buf = {"Dies ist eine Broadcast Nachricht"};
        OSMP_Bcast(buf, strlen(buf), osmp_unsigned_char, root);
    }
    else{
        unsigned int *buf = malloc(1024);
        int rv = OSMP_Bcast(buf, 0, osmp_unsigned, root);
        if(rv == OSMP_SUCCESS)printf("%u gelesen von Prozess %d\n", *buf, rank);
    }

    OSMP_Finalize();
    return 0;
}