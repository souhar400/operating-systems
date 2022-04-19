//
// Created by lw874718 on 07.04.22.
// OMSPRun creates "pr_zahl" new processes that execute "my_program" wiht the arguments "argt"
// ./OSMPRun pr_zahl my_program argt
//

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "lib/datastructs.h"
#include "osmprun.h"

#define FORKERR 10
#define EXECERR 11
#define WAITERR 12
#define SHMOPENERR 13
#define FTRUNCERR 14
#define MMAPERR 15
#define UNLINKERR 16
#define MUNMAPERR 17

int main(int argc, char *argv[]){
    errno=0;
    printf (" Die Anzahl der Argumente ist %d\n", argc);
    if (argc < 3) {
        fprintf(stderr, "\n Error: Zu wenige Argumente...\n\n");
        exit(EXIT_FAILURE);
    }

    char *endptr = NULL;
    int numProc = (int) strtol(argv[1], &endptr, 10);
    if (errno != 0 || !numProc || *endptr) {
        fprintf(stderr, "\n Error: Nicht gültige Processzahl\n\n");
        exit(EXIT_FAILURE);
    }
    if (numProc > OSMP_MAX_PROCESSES) {
        fprintf(stderr, "\n Error: Maximale Processzahl überschritten \n\n");
        exit(EXIT_FAILURE);
    }
    if (numProc < 1) {
        fprintf(stderr, "\n Error: Processzahl ist kleiner 1\n\n");
        exit(EXIT_FAILURE);
    }

    /**
     * Shared Memory
     * Command: ./OSMPRun pr_zahl my_program name_of_shared_memory
     */
    int fd;
    if ((fd = shm_open(argv[3], O_CREAT | O_EXCL | O_RDWR, 0777)) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(SHMOPENERR);
    }

    off_t memSize = (off_t) (sizeof(struct shared_memory));

    if (ftruncate(fd, memSize) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(FTRUNCERR);
    }

    void *addr = NULL;
    // Map the shared memory object.
    if ((addr = mmap(NULL, (size_t) memSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(MMAPERR);
    }

    struct shared_memory *mem = (struct shared_memory *) addr;
    mem->shm_size = memSize;
    mem->size=numProc;

    char *params[] = {argv[3], NULL};


    for (int i = 0; i < numProc; i++) {
        pid_t child_pid = fork();
        if (child_pid < (pid_t) 0) {
            fprintf(stderr, "%s\n", strerror(errno));
            exit(FORKERR);
        }
        if (child_pid == (pid_t) 0) {
            initChild(mem, i);
            execv(argv[2], params);
            fprintf(stderr, "%s\n", strerror(errno));
            exit(EXECERR);
        }
        sleep(1);
    }

    pid_t w;
    int status;
    sleep(6);
    for (int i = 0; i < numProc; ++i) {
        w = waitpid(-1, &status, WUNTRACED | WCONTINUED);

        if (w == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {//WIFEXITED: returns true if the child terminated normally, that is, by calling exit(3) or _exit(2), or by returning from main().
            printf("\nexited, status=%d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {//WIFSIGNALED: returns true if the child process was terminated by a signal.
            printf("\nkilled by signal %d\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {//WIFSTOPPED: returns true if the child process was stopped by delivery of a signal; this is only possible if the call was done using WUNTRACED or when the child is being traced (see ptrace(2)).
            printf("\nstopped by signal %d\n", WSTOPSIG(status));
        } else if (WIFCONTINUED(status)) {//WCONTINUED:(Since Linux 2.6.10) also return if a stopped child has been resumed by delivery of SIGCONT.
            printf("\ncontinued\n");
        }
    }

    if (munmap(addr, (size_t) memSize) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(MUNMAPERR);
    }

    if (shm_unlink(argv[3]) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(UNLINKERR);
    }


}

void initChild(struct shared_memory *mem, int rank)
{
    mem->processes[rank].rank = rank;
    mem->processes[rank].pid = getpid();
    mem->processes[rank].messages_zahl = OSMP_MAX_MESSAGES_PROC;
}
