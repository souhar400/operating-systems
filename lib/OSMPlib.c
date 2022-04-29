//
// Created by studi on 19.04.22.
//

#include "OSMPlib.h"


struct parameters *param;


int OSMP_Init(const int *argc, char ***argv) {
    param = malloc(sizeof(struct parameters));
    struct shared_memory *mem;
    struct stat buff;
    void *addr = NULL; // pointer to shared memory object.
    int fd; // shared memory file descriptor.

    // Check the number of arguments.
    if (*argc < 1) {
        fprintf(stderr, "\n Error: Zu wenige Argumente...\n\n");
        exit(EXIT_FAILURE);
    }

    // Init/Create the shared memory object.
    if ((fd = shm_open(OSMP_SHM_NAME, O_RDWR, 0777)) == OSMP_ERROR) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(SHMOPENERR);
    }

    // Getting the shared memory current size.
    if (fstat(fd, &buff) == OSMP_ERROR) {
        printf("[ERROR | fstat]: %s\n", strerror(errno));
        return OSMP_ERROR;
    }

    // Map the shared memory object.
    if ((addr = mmap(NULL, (size_t) buff.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        fprintf(stderr, "%s\n", strerror(errno));
        exit(MUNMAPERR);
    }

    mem = (struct shared_memory *) addr;

    param->size = (int) mem->size;
    param->shm_pointer = addr;
    pid_t pid = getpid();

    for (int i = 0; i < param->size; i++) {
        if (mem->processes[i].pid == pid) {
            param->rank = i;
            break;
        }
    }

    return OSMP_SUCCESS;
}

int OSMP_Size(int *size) {
    if (param->size <= 0) {
        return OSMP_ERROR;
    }

    *size = param->size;
    return OSMP_SUCCESS;
}


int OSMP_Rank(int *rank) {
    if (param->rank < 0) {
        return OSMP_ERROR;
    }
    *rank = param->rank;
    return OSMP_SUCCESS;

}

int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest) {

    int my_msg_index = param->shm_pointer->actual_free_slot;
    struct message *my_msg_slot = &param->shm_pointer->messages[my_msg_index];
    param->shm_pointer->actual_free_slot = my_msg_slot->next_free_msg_slot;
    printf("\n Slot wo die Nachricht geschrieben wird ist %d\n ", my_msg_index);
    my_msg_slot->sender_pr_rank =  param->rank;
    my_msg_slot->elt_zahl =count;
    my_msg_slot->msg_len = count* sizeof(datatype);
    my_msg_slot->elt_datentyp= datatype;
    memcpy(my_msg_slot->payload, buf, my_msg_slot->msg_len);

    param->shm_pointer->processes[dest].msg_slot[param->shm_pointer->processes[dest].write_index] = my_msg_index;
    param->shm_pointer->processes[dest].write_index++;

    return 0;
}

int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len) {

    int msg_slot = param->shm_pointer->processes[param->rank].msg_slot[param->shm_pointer->processes[param->rank].read_index];
    param->shm_pointer->processes[param->rank].read_index++;

    printf("\n der Slot wo die zu empfangennen Nachricht liegt ist : %d\n ", msg_slot);


    struct message *my_msg_slot = &param->shm_pointer->messages[msg_slot];

    param->shm_pointer->messages[msg_slot].next_free_msg_slot = param->shm_pointer->actual_free_slot;
    param->shm_pointer->actual_free_slot= msg_slot;

    *source = my_msg_slot->sender_pr_rank;
    *len = my_msg_slot->msg_len;

    memcpy(buf, my_msg_slot->payload, *len);

    return 0;
}

int OSMP_Finalize(void) {

    struct shared_memory *mem = (struct shared_memory *) param->shm_pointer;
    if (munmap(param->shm_pointer, (size_t) mem->shm_size) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        return OSMP_ERROR;
    }
    return OSMP_SUCCESS;
}
void OSMP_wait(sem_t *sem){
    int rv = sem_wait(sem);
    if(rv == -1){
        ERROR_ROUTINE(SEMERR)
    }
}
void OSMP_signal(sem_t *sem){
    int rv = sem_post(sem);
    if(rv == -1){
        ERROR_ROUTINE(SEMERR)
    }
}