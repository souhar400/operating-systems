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

    //general_actual_free_slot in shm->messages[] HOLEN und UPDATEN
    // --> LESEN und SCHREIBEN in "shm" und  "shm->messages[]" <--
    OSMP_wait(&param->shm_pointer->free_slots);
    OSMP_wait(&param->shm_pointer->shm_mutex);
        int my_msg_index = param->shm_pointer->actual_free_slot;
        struct message *my_msg_instance = &param->shm_pointer->messages[my_msg_index];
        param->shm_pointer->actual_free_slot = my_msg_instance->next_free_msg_slot;
        //my_msg_instance VORBEREITEN
        // --> Schreiben  in "shm->messages[FEST]" <--
        my_msg_instance->sender_pr_rank =  param->rank;
        my_msg_instance->elt_zahl =count;
        my_msg_instance->msg_len = (int) (count * sizeof(datatype));
        my_msg_instance->elt_datentyp= datatype;
        memcpy(my_msg_instance->payload, buf, (unsigned long) my_msg_instance->msg_len);
    OSMP_signal(&param->shm_pointer->shm_mutex);
    OSMP_signal(&param->shm_pointer->belegte_slots);


    OSMP_wait(&dest_process->freie_slots);
    OSMP_wait(&dest_process->proc_mutex);
        //daten in process[dest] UPDATEN: in process[]->msg_slots[write_index] my_msg_index SCHREIBEN und write_index UPDATEN
        // --> LESEN und SCHREIBEN in "shm->processes[]" <--
        dest_process->write_index = dest_process->write_index % OSMP_MAX_MESSAGES_PROC;
        //sicherstellen dass es Platz gibt : ohne -1 Methode, durch Semaphore ?
        dest_process->msg_slots[dest_process->write_index] = my_msg_index;
        dest_process->write_index++;
    OSMP_signal(&dest_process->proc_mutex);
    OSMP_signal(&dest_process->belegte_slots);

    return 0;
}

int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len) {
    struct process *reciever_process = &param ->shm_pointer->processes[param->rank];
    int sem_value;
    sem_getvalue(&reciever_process->belegte_slots, &sem_value);


    OSMP_wait(&reciever_process->belegte_slots);
        //OSMP_wait(&reciever_process->proc_mutex);
        //msg_slot HOLEN [»] read_index && msg_slots[read_index] UPDATEN : from shm_process[][]
        //--> LESEN und SCHREIBEN in "shm->processes[]" <--
        reciever_process->read_index = reciever_process->read_index % OSMP_MAX_MESSAGES_PROC;
        int msg_slot = reciever_process->msg_slots[reciever_process->read_index];
        //reciever_process->msg_slots[reciever_process->read_index] =-1;
        reciever_process->read_index++;

        //nächste frei stellen UPDATEN
        // --> LESEN und SCHREIBEN in "shm" und  "shm->messages[]" <--
        struct message *my_msg_instance = &param->shm_pointer->messages[msg_slot];
        param->shm_pointer->messages[msg_slot].next_free_msg_slot = param->shm_pointer->actual_free_slot;
        OSMP_wait(&param->shm_pointer->shm_mutex);
        param->shm_pointer->actual_free_slot= msg_slot;
        OSMP_signal(&param->shm_pointer->shm_mutex);

        //my_msg_instance UPDATEN : [FEST]
        *source = my_msg_instance->sender_pr_rank;
        *len = my_msg_instance->msg_len;
        //Nachricht HOLEN
        memcpy(buf, my_msg_instance->payload, *len);
    OSMP_signal(&param->shm_pointer->free_slots);
    //OSMP_signal(&reciever_process->proc_mutex);
    OSMP_signal(&reciever_process->freie_slots);

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