//
// Created by lw874718 on 07.04.22.
//

#include "lib/OSMPlib.h"

int main(int argc, char *argv[]) {
    int rank = 0, size = 0;
    int source, len;
    OSMP_Init(&argc, &argv);
    OSMP_Rank(&rank);
    OSMP_Size(&size);
    printf("RANK %d \n", rank);
    printf("Size %d \n\n", size);

    // 0 --> 1
    if(rank == 0) {
        printf("\n der Prozess %d läuft nun\n ", rank);
        char* bufin = "hey diese Nachricht ist von 0 an 1";

        // Rank 0 -> Rank 1
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 1);
    }

    // 2--> 1
    if(rank == 2) {
        printf("\n der Prozess %d läuft nun\n ", rank);
        char* bufin = "hey diese Nachricht ist von 2 an 1";

        // Rank 0 -> Rank 1
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 1);
    }

    //Read rank 1 messages
    if(rank == 1) {
        sleep(2);
        printf("\n der Prozess %d läuft nun\n", rank);

        char *bufout = calloc(512, 1);
        int source, len;
        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);
    }

    // 3--> 4
    if(rank == 3) {
        sleep(5);
        printf("\n der Prozess %d läuft nun\n ", rank);
        char* bufin = "hey diese Nachricht ist von 3 an 4";

        // Rank 0 -> Rank 1
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 4);
    }


    //Read rank 4 messages
    if(rank == 4) {
        sleep(7);
        printf("\n der Prozess %d läuft nun\n", rank);

        char *bufout = calloc(512, 1);
        int source, len;
        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);
        char* bufin = "hey diese Nachricht ist von 4 an 0";
        char* bufin1 = "hey diese Nachricht ist von 4 an 2";

        sleep(1);
        //4 -> 0
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 0);

        sleep(1);
        //4-> 2
        OSMP_Send(bufin,strlen(bufin1), osmp_unsigned_char, 2);

    }

    OSMP_Finalize();
    return 0;
}
