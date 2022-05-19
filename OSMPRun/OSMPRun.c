//
// Created by lw874718 on 07.04.22.
// OMSPRun creates "pr_zahl" new processes that execute "my_program" wiht the arguments "argt"
// ./OSMPRun pr_zahl my_program argt
//

/**
 * GDB CHEAT SHEET
 * set detach-on-fork off
 * set follow-fork-mode child
 */

#include "lib/OSMP.h"
#include "osmprun.h"



int main(int argc, char *argv[]){
    errno=0;
    printf ("Die Anzahl der Argumente ist %d\n", argc);

    if (argc < 2) {
        ERROR_ROUTINE( EXIT_FAILURE)
    }
    char *endptr = NULL;
    int numProc = (int) strtol(argv[1], &endptr, 10);
    if (errno != 0 || !numProc || *endptr) {
        ERROR_ROUTINE(EXIT_FAILURE)
    }
    if (numProc > OSMP_MAX_PROCESSES) {
        ERROR_ROUTINE(EXIT_FAILURE)
    }
    if (numProc < 1) {
        ERROR_ROUTINE(EXIT_FAILURE)
    }

    /**
     * Shared Memory
     * Command: ./OSMPRun pr_zahl my_program name_of_shared_memory
     */
    int fd;
    if ((fd = shm_open(OSMP_SHM_NAME, O_CREAT | O_RDWR, 0777)) == -1) {
        ERROR_ROUTINE(SHMOPENERR)
    }

    off_t memSize = (off_t) (sizeof(struct shared_memory));

    if (ftruncate(fd, memSize) == -1) {
        ERROR_ROUTINE(FTRUNCERR)
    }

    void *addr = NULL;
    // Map the shared memory object.
    if ((addr = mmap(NULL, (size_t) memSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        ERROR_ROUTINE(MMAPERR)
    }

    struct shared_memory *mem = (struct shared_memory *) addr;
    mem->shm_size = memSize;
    mem->size=numProc;
    initMemory(mem);
    char *params[] = {OSMP_SHM_NAME,"",NULL};

    for (int i = 0; i < numProc; i++) {
        char p[get_digit_count(i) + 1];
        sprintf(p, "%d", i);
        params[1] = p;
        pid_t child_pid = fork();
        if (child_pid < (pid_t) 0) {
            ERROR_ROUTINE(FORKERR)
        }
        if (child_pid == (pid_t) 0) {
            initChild(mem, i);
            execv(argv[2], params);
            ERROR_ROUTINE(EXECERR)
        }
        sleep(1);
    }

    pid_t w;
    int status;
    sleep(6);
    for (int i = 0; i < numProc; ++i) {
        w = waitpid(-1, &status, WUNTRACED | WCONTINUED);

        if (w == -1) {
            ERROR_ROUTINE(WAITERR)
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
        ERROR_ROUTINE(MUNMAPERR)
    }

    if (shm_unlink(OSMP_SHM_NAME) == -1) {
        ERROR_ROUTINE(UNLINKERR)
    }


}


void initChild(struct shared_memory *mem, int rank)
{

    struct process *proc = &mem->processes[rank];
    proc->rank = rank; //<=>     (*mem).processes[rank].rank=rank;    (*mem).processes[rank].rank=rank;
    proc->pid = getpid(); // <=>     mem->processes[rank].pid=getpid();
    sem_init(&proc->proc_mutex, 1, 1);
    sem_init(&proc->proc_full, 1, 0);
    sem_init(&proc->proc_free, 1, OSMP_MAX_MESSAGES_PROC);
}

void initMemory(struct shared_memory *mem){
    //sem_init(&mem->belegte_slots, 1, 0);
    sem_init(&mem->free_slots, 1, OSMP_MAX_SLOTS-1);
    sem_init(&mem->shm_mutex, 1 , 1);
    //Broadcast mit semaphoren
    //sem_init(&mem->barrier.sem_barrier, 1, 0);
    //mem->barrier.count = 0;

    //Broadcast mit pthread_barrier
    //pthread_barrierattr_t attr;
    //pthread_barrierattr_init(&attr);
    //pthread_barrierattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    //pthread_barrier_init(&mem->barrier, &attr, (unsigned int) mem->size);

    //Broadcast mit Condition Variables
    pthread_mutexattr_t mutexattr;
    pthread_condattr_t condattr;

    pthread_mutexattr_init(&mutexattr);
    pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mem->cond_barrier.bcast_mutex, &mutexattr);

    pthread_condattr_init(&condattr);
    pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&mem->cond_barrier.bcast_cond, &condattr);

    mem->cond_barrier.cond = mem->size;

    for(int i = 0; i < OSMP_MAX_SLOTS-1; i++){
        mem->messages[i].next_free_msg_slot = (i+1) % (OSMP_MAX_SLOTS-1) ;
    }

    for (int i=0; i < mem->size; i++){
        mem->processes[i].read_index= 0;
        mem->processes[i].write_index= 0;

    }
    mem->actual_free_slot=0;
}
int get_digit_count(int size){
    int count = 0;
    do{
        size /=10;
        count++;
    }while(size > 0);
    return count;
}