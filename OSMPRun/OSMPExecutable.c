//
// Created by lw874718 on 07.04.22.
//

#include "lib/OSMPlib.h"

int main(int argc, char *argv[]) {
    int rank = 0, size = 0;
    OSMP_Init(&argc, argv);
    OSMP_Rank(&rank);
    OSMP_Size(&size);
    //printf("RANK %d \n", rank);
    //printf("Size %d \n\n", size);

    // 0 --> 1
    // read 1 msg from 1
    if(rank == 0) {
        printf("der Prozess %d läuft nun...\n ", rank);

        int source, len;
        char *bufout = calloc(512, 1);
        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);

        char* bufin = "hey diese Nachricht ist von 0 an 1";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 1);
        printf("Senden beendet von 0 an 1\n");

    }


    // 2--> 1
    //read one msg from 1, 5
    if(rank == 2) {
        printf("der Prozess %d läuft nun\n", rank);

        char* bufin = "hey diese Nachricht ist von 2 an 1";
        char *bufout = calloc(512, 1);
        int source, len;

        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n ", rank, len, source, bufout);


        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);

        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 1);
        printf("Senden beendet von 2 an 1\n");
    }

    // 3 --> 1 : HALLO
    if(rank == 3) {
        printf("der Prozess %d läuft nun ...\n ", rank);
        char* bufin = "von 3 an 1: H";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 1);

        bufin = "von 3 an 1: A";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 1);

        bufin = "von 3 an 1: L";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 1);

        bufin = "von 3 an 1: L";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 1);

        bufin = "von 3 an 1: O";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 1);


        printf("Senden beendet von 3 an 1\n");

    }

    // 4 --> 1
    // read 1 msg from 5
    if(rank == 4) {
        char *bufout = calloc(512, 1);
        int source, len;

        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);

        printf("der Prozess %d läuft nun...\n ", rank);
        char* bufin = "hey diese Nachricht ist von 4 an 1";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 1);
        printf("Senden beendet von 4 an 1\n");

    }
    // 5 --> 1
    //5--> 4
    //5-->2
    if(rank == 5) {
        printf("der Prozess %d läuft nun...\n", rank);
        char* bufin = "hey diese Nachricht ist von 5 an 1";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 1);
        printf("Senden beendet von 5 an 1\n");

        bufin = "hey diese Nachricht ist von 5 an 4";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 4);
        printf("Senden beendet von 5 an 4\n");

        bufin = "hey diese Nachricht ist von 5 an 2";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 2);
        printf("Senden beendet von 5 an 2\n");

    }


//1 read all messages :5msg from 3, 1msg from 2, 1msg from 5, 1msg from 0, 1msg from 4
//1 -> 2
//1 -> 0
    if(rank == 1) {
        sleep(3);
        printf(" der Prozess %d läuft nun\n", rank);

        char *bufout = calloc(512, 1);
        int source, len;
        char* bufin = "hey diese Nachricht ist von  1 an 2";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 2);

        bufin = "hey diese Nachricht ist von  1 an 0";
        OSMP_Send(bufin,strlen(bufin), osmp_unsigned_char, 0);


        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);
        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);

        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);
        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);

        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);
        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);

        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);
        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);

        OSMP_Recv(bufout, 64, osmp_unsigned_char, &source, &len);
        printf("OSMP process %d received %d byte from %d [%s] \n", rank, len, source, bufout);
    }

    OSMP_Finalize();
    return 0;
}
