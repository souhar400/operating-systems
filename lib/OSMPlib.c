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
    struct process *dest_process = &param->shm_pointer->processes[dest];
    dest_process->write_index = dest_process->write_index % OSMP_MAX_MESSAGES_PROC;

    //general_actual_free_slot HOLEN und UPDATEN
    int my_msg_index = param->shm_pointer->actual_free_slot;
    struct message *my_msg_instance = &param->shm_pointer->messages[my_msg_index];
    param->shm_pointer->actual_free_slot = my_msg_instance->next_free_msg_slot;
    //my_msg_instance VORBEREITEN
    printf("\n Slot wo die Nachricht geschrieben wird ist %d\n ", my_msg_index);
    my_msg_instance->sender_pr_rank =  param->rank;
    my_msg_instance->elt_zahl =count;
    my_msg_instance->msg_len = (int) (count * sizeof(datatype));
    my_msg_instance->elt_datentyp= datatype;
    memcpy(my_msg_instance->payload, buf, (unsigned long) my_msg_instance->msg_len);
    //daten in process[dest] UPDATEN: in msg_slots[write] my_msg_index SCHREIBEN und write_index UPDATEN
    dest_process->msg_slots[dest_process->write_index] = my_msg_index;
    dest_process->write_index++;

    return 0;
}

int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len) {

    struct process *reciever_process = &param ->shm_pointer->processes[param->rank];
    //msg_slot HOLEN [»] read_index && msg_slots[read_index] UPDATEN
    reciever_process->read_index = reciever_process->read_index % OSMP_MAX_MESSAGES_PROC;
    int msg_slot = reciever_process->msg_slots[reciever_process->read_index];
    reciever_process->msg_slots[reciever_process->read_index] =-1;
    reciever_process->read_index++;

    printf("\n der Slot, wo die Nachricht zu empfangen liegt ist : %d\n ", msg_slot);


    struct message *my_msg_instance = &param->shm_pointer->messages[msg_slot];
    //nächste frei stellen UPDATEN
    param->shm_pointer->messages[msg_slot].next_free_msg_slot = param->shm_pointer->actual_free_slot;
    param->shm_pointer->actual_free_slot= msg_slot;
    //my_msg_instance UPDATEN
    *source = my_msg_instance->sender_pr_rank;
    *len = my_msg_instance->msg_len;
    //Nachricht HOLEN
    memcpy(buf, my_msg_instance->payload, *len);
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