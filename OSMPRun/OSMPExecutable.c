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
#define FORKERR 10
#define EXECERR 11
#define WAITERR 12
#define SHMOPENERR 13
#define FTRUNCERR 14
#define MMAPERR 15
#define UNLINKERR 16
#define MUNMAPERR 17

int main(){
    int fd, rv;
    char buf[1024];
    fd = shm_open("/shared_memory", O_RDWR, 0777);
    void * ptr=NULL;
    if((ptr= mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd, 0)) == MAP_FAILED){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(MMAPERR);
    }

    printf("%s\n", (char *) ptr);

    if((rv =munmap(ptr, 4096)) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(MUNMAPERR);
    }

}