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
    int fd, rv, wstatus;
    pid_t pid, wpid;
    char buf[1024] = "Hello World from Parent\n";
    fd = shm_open("/shared_memory", O_CREAT | O_EXCL | O_RDWR, 0777);
    if(fd < 0){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(SHMOPENERR);
    }
    if(( rv = ftruncate(fd, 4096)) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(FTRUNCERR);
    }
    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(FORKERR);
    }
    void * ptr=NULL;
    if (pid == 0) {
        execl("/home/ms/l/lw874718/CLionProjects/grp28/bin/OSMPExecutable",
              "etwas", NULL);
        fprintf(stderr, "%s\n", strerror(errno));
        exit(EXECERR);
    }
    if(( ptr= mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd, 0)) == MAP_FAILED){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(MMAPERR);
    }
    memcpy(ptr, buf, 1024);
    while((wpid = wait(&wstatus)) > 0) {
        if (wpid == -1) {
            fprintf(stderr, "%s\n", strerror(errno));
            exit(WAITERR);
        }
        printf("Child: %d exited with status: %d\n", wpid, WEXITSTATUS(wstatus));
    }
    if((rv =munmap(ptr, 4096)) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(MUNMAPERR);
    }
    if((rv =shm_unlink("/shared_memory")) == -1){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(UNLINKERR);
    }
}