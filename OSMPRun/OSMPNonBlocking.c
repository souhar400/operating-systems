//
// Created by etiuser on 19.05.22.
//
#include "lib/OSMPlib.h"
int main(int argc, char** argv){
    int rank = 0, size = 0;
    OSMP_Init(&argc, argv);
    OSMP_Rank(&rank);
    OSMP_Size(&size);

    //1 send to 2
    //1 send to 0
    if(rank == 1) {
        sleep(3);
        printf(" der Prozess %d läuft nun\n", rank);

        OSMP_Request myrequest;
        OSMP_CreateRequest(&myrequest);

        char *bufin = "hey diese Nachricht ist von  1 an 2";
        OSMP_Isend(bufin, strlen(bufin), osmp_unsigned_char, 2, myrequest);
        OSMP_Wait(myrequest);
        bufin = "hey diese Nachricht ist von  1 an 0";
        OSMP_Isend(bufin, strlen(bufin), osmp_unsigned_char, 0, myrequest);
        OSMP_Wait(myrequest);
}
    // read 1 msg from 1
    if(rank == 0) {
        printf("der Prozess %d läuft nun...\n ", rank);
        OSMP_Request myrequest;
        OSMP_CreateRequest(&myrequest);

        int source, len;
        char *bufout = calloc(512, 1);
        OSMP_Irecv(bufout, 64, osmp_unsigned_char, &source, &len,myrequest);
        OSMP_Wait(myrequest);

        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);
    }

    //read one msg from 1
    if(rank == 2) {
        printf("der Prozess %d läuft nun\n", rank);

        OSMP_Request myrequest;
        OSMP_CreateRequest(&myrequest);

        char *bufout = calloc(512, 1);
        int source, len;

        OSMP_Irecv(bufout, 64, osmp_unsigned_char, &source, &len,myrequest);
        OSMP_Wait(myrequest);

        printf("OSMP process %d received %d byte from %d [%s] \n ", rank, len, source, bufout);

    }

    OSMP_Finalize();
    return 0;
}