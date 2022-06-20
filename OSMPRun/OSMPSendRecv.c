#include "lib/OSMPlib.h"

int main(int argc, char *argv[]){
    int rank = 0, size = 0;
    int source, len;
    OSMP_Init(&argc, argv);
    OSMP_Rank(&rank);
    OSMP_Size(&size);
    char *buf = malloc(1024);
    OSMP_Recv(buf, 64, osmp_unsigned_char, &source, &len);
    printf("sollte nicht passieren\n");
    return 0;
}