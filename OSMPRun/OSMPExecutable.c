//
// Created by lw874718 on 07.04.22.
//
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "lib/OSMPlib.h"

int main(int argc, char *argv[]) {
    int rank = 0, size = 0;
    OSMP_Init(&argc, &argv);
    OSMP_Rank(&rank);
    OSMP_Size(&size);
    printf("RANK %d \n", rank);
    printf("Size %d \n\n", size);
    OSMP_Finalize();
    return 0;
}
