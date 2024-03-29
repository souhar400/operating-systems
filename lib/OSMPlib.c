﻿//
// Created by studi on 19.04.22.
//

#include "OSMPlib.h"


struct parameters *param;
int setup =-1;
#define shm_handle param->shm_pointer

int OSMP_Init(const int *argc, char **argv) {
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
    param->rank = (int) strtol(argv[1], NULL, 10);
    setup= 1;
    return OSMP_SUCCESS;
}

int OSMP_Size(int *size) {
    if( setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }

    if (param->size <= 0) {
        return OSMP_ERROR;
    }

    *size = param->size;
    return OSMP_SUCCESS;
}


int OSMP_Rank(int *rank) {
    if( setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }

    if (param->rank < 0 )
        return OSMP_ERROR;

    *rank = param->rank;
    return OSMP_SUCCESS;

}

int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest) {

    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    struct process *dest_process = &param->shm_pointer->processes[dest];



    //prüfen ob destination ist gültig
    if(dest >= param->size || dest<0 || dest == param->rank) {
        fprintf(stderr, "Error: Destination not found or invalid\n");
        return OSMP_ERROR;
    }

    //prüfen maximales Payload
    if(datatype* (unsigned long) count> OSMP_MAX_PAYLOAD_LENGTH ) {
        fprintf(stderr, "Error: Maximum data length exceeded\n");
        return OSMP_ERROR;
    }


    //general_actual_free_slot in shm->messages[] HOLEN und UPDATEN
    // --> LESEN und SCHREIBEN in "shm" und  "shm->messages[]" <--
    OSMP_sem_wait(&dest_process->proc_free);
    OSMP_sem_wait(&param->shm_pointer->free_slots);
    OSMP_sem_wait(&param->shm_pointer->shm_mutex);
        int my_msg_index = param->shm_pointer->actual_free_slot;
        struct message *my_msg_instance = &param->shm_pointer->messages[my_msg_index];
        param->shm_pointer->actual_free_slot = my_msg_instance->next_free_msg_slot;
    OSMP_signal(&param->shm_pointer->shm_mutex);
        //my_msg_instance VORBEREITEN
        // --> Schreiben  in "shm->messages[FEST]" <--

        my_msg_instance->sender_pr_rank =  param->rank;
        my_msg_instance->elt_zahl =count;
        my_msg_instance->msg_len = (int) (count * datatype);
        my_msg_instance->elt_datentyp= datatype;
        memcpy(my_msg_instance->payload, buf, (unsigned long) my_msg_instance->msg_len);


    OSMP_sem_wait(&dest_process->proc_mutex);
        //daten in process[dest] UPDATEN: in process[]->msg_slots[write_index] my_msg_index SCHREIBEN und write_index UPDATEN
        // --> LESEN und SCHREIBEN in "shm->processes[]" <--
        dest_process->write_index = dest_process->write_index % OSMP_MAX_MESSAGES_PROC;
        //sicherstellen dass es Platz gibt : ohne -1 Methode, durch Semaphore ?
        dest_process->msg_slots[dest_process->write_index] = my_msg_index;
        dest_process->write_index++;
    OSMP_signal(&dest_process->proc_mutex);
    OSMP_signal(&dest_process->proc_full);

    return 0;
}

int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len) {
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    struct process *reciever_process = &param ->shm_pointer->processes[param->rank];
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }

    OSMP_sem_wait(&reciever_process->proc_full);
        //OSMP_sem_wait(&reciever_process->proc_mutex);
        //msg_slot HOLEN [»] read_index && msg_slots[read_index] UPDATEN : from shm_process[][]
        //--> LESEN und SCHREIBEN in "shm->processes[]" <--
        reciever_process->read_index = reciever_process->read_index % OSMP_MAX_MESSAGES_PROC;
        int msg_slot = reciever_process->msg_slots[reciever_process->read_index];
        if(param->shm_pointer->messages[msg_slot].elt_datentyp != datatype){
            fprintf(stderr, "wrong Datatype on receive");
            return OSMP_ERROR;
        }
        //reciever_process->msg_slots[reciever_process->read_index] =-1;
        reciever_process->read_index++;

        //nächste frei stellen UPDATEN
        // --> LESEN und SCHREIBEN in "shm" und  "shm->messages[]" <--
        struct message *my_msg_instance = &param->shm_pointer->messages[msg_slot];
    OSMP_sem_wait(&param->shm_pointer->shm_mutex);
            param->shm_pointer->messages[msg_slot].next_free_msg_slot = param->shm_pointer->actual_free_slot;
            param->shm_pointer->actual_free_slot= msg_slot;
        OSMP_signal(&param->shm_pointer->shm_mutex);

        if(param->shm_pointer->messages[msg_slot].msg_len <= (datatype * (unsigned long) count)) {
            //my_msg_instance UPDATEN : [FEST]
            *source = my_msg_instance->sender_pr_rank;
            *len = my_msg_instance->msg_len;
            //Nachricht HOLEN
            memcpy(buf, my_msg_instance->payload, *len);
        }
        else {
            fprintf(stderr, "Error: Message bigger than allowed payload\n");
            return OSMP_ERROR;
        }

    OSMP_signal(&param->shm_pointer->free_slots);
    //OSMP_signal(&reciever_process->proc_mutex);
    OSMP_signal(&reciever_process->proc_free);

    return 0;
}

int OSMP_Finalize(void) {
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    struct shared_memory *mem = (struct shared_memory *) param->shm_pointer;
    OSMP_sem_wait(&mem->shm_mutex);
    mem->size--;
    OSMP_signal(&mem->shm_mutex);
    if (munmap(param->shm_pointer, (size_t) mem->shm_size) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        return OSMP_ERROR;
    }
    return OSMP_SUCCESS;
}
void OSMP_sem_wait(sem_t *sem){
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return;
    }
    int rv = sem_wait(sem);
    if(rv == -1){
        ERROR_ROUTINE(SEMERR)
    }
}
void OSMP_signal(sem_t *sem){
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return;
    }
    int rv = sem_post(sem);
    if(rv == -1){
        ERROR_ROUTINE(SEMERR)
    }

}
int OSMP_Barrier(){
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    int rv;
    pthread_mutex_lock(&shm_handle->cond_barrier.bcast_mutex);
    if(shm_handle->size <  shm_handle->cond_barrier.cond){
        shm_handle->cond_barrier.cond = shm_handle->size;
    }
    shm_handle->cond_barrier.cond--;

    if(shm_handle->cond_barrier.cond == 0){
        shm_handle->cond_barrier.cond = shm_handle->size;
        rv = pthread_mutex_unlock(&shm_handle->cond_barrier.bcast_mutex);
        if(rv != 0) ERROR_ROUTINE(BARRIER_ERROR)
        rv = pthread_cond_broadcast(&shm_handle->cond_barrier.bcast_cond);
        if(rv != 0) ERROR_ROUTINE(BARRIER_ERROR)

    }
    else {
        rv = pthread_cond_wait(&shm_handle->cond_barrier.bcast_cond, &shm_handle->cond_barrier.bcast_mutex);
        if(rv != 0) ERROR_ROUTINE(BARRIER_ERROR)
        rv = pthread_mutex_unlock(&shm_handle->cond_barrier.bcast_mutex);
        if(rv != 0) ERROR_ROUTINE(BARRIER_ERROR)
    }
    return OSMP_SUCCESS;
}

int OSMP_Bcast(void *buf, int count, OSMP_Datatype datatype, int root){
    if(param->rank == root){
        if(setup==-1) {
            fprintf(stderr, "Process : Shared memory is not initialized");
            return OSMP_ERROR;
        }
        OSMP_sem_wait(&shm_handle->shm_mutex);


        struct message *msg = &param->shm_pointer->messages[OSMP_BCAST_SLOT];
        msg->sender_pr_rank = root;
        msg->elt_datentyp = datatype;
        msg->elt_zahl = count;
        msg->msg_len = (int) ((unsigned int) count * datatype);
        memcpy(msg->payload, buf, (unsigned long) msg->msg_len);
        OSMP_signal(&param->shm_pointer->shm_mutex);

        OSMP_Barrier();
    }
    else{

        OSMP_Barrier();
        struct message *msg = &param->shm_pointer->messages[OSMP_BCAST_SLOT];
        if(msg->elt_datentyp != datatype){
            fprintf(stderr, "Falscher Datentyp\n");
            OSMP_Barrier();
            return OSMP_ERROR;
        }
        void* rv;
        rv = realloc(buf, (unsigned long) msg->msg_len);
        if(rv == NULL) ERROR_ROUTINE(OSMP_ERROR)
        memcpy(buf, msg->payload, (unsigned long) msg->msg_len);
    }
    OSMP_Barrier(); // Synchronizieren auf Nachrichten gelesen, falls mehrere Aufrufe sicherstellen das letzter Bcasat aufruf durch ist
    return OSMP_SUCCESS;
}

int OSMP_CreateRequest(OSMP_Request *request) {
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    *request = malloc (sizeof( request_data));
    return OSMP_SUCCESS;
}

int OSMP_RemoveRequest(OSMP_Request *request) {
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    free(*request);
    return OSMP_SUCCESS;
}

void* thread_send( void* arg){
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    OSMP_Request myrequest = (OSMP_Request ) arg;
    myrequest->status= inprogress;
    int rv = OSMP_Send(myrequest->buf, myrequest->count, myrequest->datatype, myrequest->dest);
    myrequest->retVal = rv;
    myrequest->status=done;
    pthread_exit(OSMP_SUCCESS);
}

void* thread_recv( void* arg){
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    OSMP_Request myrequest = (OSMP_Request ) arg;
    myrequest->status= inprogress;
    int rv = OSMP_Recv(myrequest->buf, myrequest->count, myrequest->datatype, myrequest->source,  myrequest->len);
    myrequest->retVal = rv;
    myrequest->status=done;
    pthread_exit(OSMP_SUCCESS);
}

int OSMP_Isend(const void *buf, int count, OSMP_Datatype datatype, int dest, OSMP_Request request){
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    request->buf = (void *)buf;
    request->count = count;
    request->datatype = datatype;
    request->dest = dest;
    pthread_create(&request->tid, NULL, thread_send, request);
    return OSMP_SUCCESS;
}

int OSMP_Irecv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len, OSMP_Request request)  {
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    request->buf = buf;
    request->count = count;
    request->datatype = datatype;
    request->source = source;
    request->len = len;

    pthread_create( &request->tid, NULL, thread_recv, request);
    return OSMP_SUCCESS;
}

int OSMP_Test(OSMP_Request request, int *flag){
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    *flag= request->status;
    return OSMP_SUCCESS;
}

int OSMP_Wait(OSMP_Request request) {
    if(setup==-1) {
        fprintf(stderr, "Process : Shared memory is not initialized");
        return OSMP_ERROR;
    }
    if(pthread_join(request->tid, NULL) || request->retVal == OSMP_ERROR)
        return OSMP_ERROR;
    return OSMP_SUCCESS;
}






















