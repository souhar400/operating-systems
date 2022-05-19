//
// Created by studi on 19.04.22.
//

#include "OSMPlib.h"


struct parameters *param;
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
    struct process *reciever_process = &param ->shm_pointer->processes[param->rank];

    OSMP_sem_wait(&reciever_process->proc_full);
        //OSMP_sem_wait(&reciever_process->proc_mutex);
        //msg_slot HOLEN [»] read_index && msg_slots[read_index] UPDATEN : from shm_process[][]
        //--> LESEN und SCHREIBEN in "shm->processes[]" <--
        reciever_process->read_index = reciever_process->read_index % OSMP_MAX_MESSAGES_PROC;
        int msg_slot = reciever_process->msg_slots[reciever_process->read_index];
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

    struct shared_memory *mem = (struct shared_memory *) param->shm_pointer;
    if (munmap(param->shm_pointer, (size_t) mem->shm_size) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        return OSMP_ERROR;
    }
    return OSMP_SUCCESS;
}
void OSMP_sem_wait(sem_t *sem){
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
int OSMP_Barrier(){
    //Broadcast mit semaphoren
//    struct shared_memory *shm = param->shm_pointer;
//    if(shm->barrier.count != shm->size) shm->barrier.count++;
//
//    if(shm->barrier.count == shm->size){
//        OSMP_signal(&shm->barrier.sem_barrier);
//    }
//    else {
//        OSMP_sem_wait(&shm->barrier.sem_barrier);
//        OSMP_signal(&shm->barrier.sem_barrier);
//    }

    //Broadcast mit pthread_barrier
    //pthread_barrier_wait(&param->shm_pointer->barrier);

    //Broadcast mit Condition Variables

    pthread_mutex_lock(&shm_handle->cond_barrier.bcast_mutex);
    shm_handle->cond_barrier.cond--;
    if(shm_handle->cond_barrier.cond == 0){
        pthread_cond_broadcast(&shm_handle->cond_barrier.bcast_cond);
    }
    else {
        pthread_cond_wait(&shm_handle->cond_barrier.bcast_cond, &shm_handle->cond_barrier.bcast_mutex);
    }
    shm_handle->cond_barrier.cond = shm_handle->size;
    pthread_mutex_unlock(&shm_handle->cond_barrier.bcast_mutex);
    return OSMP_SUCCESS;
}

int OSMP_Bcast(void *buf, int count, OSMP_Datatype datatype, int root){
    if(param->rank == root){
        OSMP_sem_wait(&shm_handle->shm_mutex);


        struct message *msg = &param->shm_pointer->messages[OSMP_BCAST_SLOT];
        msg->sender_pr_rank = root;
        msg->elt_datentyp = datatype;
        msg->elt_zahl = count;
        msg->msg_len = (int) ((unsigned int) count * datatype);
        memcpy(msg->payload, buf, (unsigned long) msg->msg_len);
        OSMP_signal(&param->shm_pointer->shm_mutex);

        OSMP_Barrier();



        //Broadcast mit pthread_barrier
        //pthread_barrier_wait(&param->shm_pointer->barrier);
    }
    else{
        //Broadcast mit pthread_barrier
        //pthread_barrier_wait(&param->shm_pointer->barrier);
        OSMP_Barrier();
        struct message *msg = &param->shm_pointer->messages[OSMP_BCAST_SLOT];
        realloc(buf, (unsigned long) msg->msg_len);
        memcpy(buf, msg->payload, (unsigned long) msg->msg_len);
    }
    OSMP_Barrier(); // Synchronizieren auf Nachrichten gelesen, falls mehrere Aufrufe sicherstellen das letzter Bcasat aufruf durch ist
    return OSMP_SUCCESS;
}

int OSMP_CreateRequest(OSMP_Request *request) {
    *request = malloc (sizeof( request_data));
    return OSMP_SUCCESS;
}

int OSMP_RemoveRequest(OSMP_Request *request) {
    free(*request);
    return OSMP_SUCCESS;
}

void* thread_send( void* arg){

    OSMP_Request myrequest = (OSMP_Request ) arg;
    myrequest->status= inprogress;
    OSMP_Send(myrequest->buf, myrequest->count, myrequest->datatype, myrequest->dest);
    myrequest->status=done;
    pthread_exit(OSMP_SUCCESS);
}

void* thread_recv( void* arg){
    OSMP_Request myrequest = (OSMP_Request ) arg;
    myrequest->status= inprogress;
    printf("\n from thread: i will start with recieving the msg\n");

    OSMP_Recv(myrequest->buf, myrequest->count, myrequest->datatype, myrequest->source,  myrequest->len);
    printf("\n from thread: im done with recieving the msg\n");
    myrequest->status=done;
    pthread_exit(OSMP_SUCCESS);
}

int OSMP_Isend(const void *buf, int count, OSMP_Datatype datatype, int dest, OSMP_Request request){
    request->buf = buf;
    request->count = count;
    request->datatype = datatype;
    request->dest = dest;
    printf("\ndest ist : %d\n", request->dest);
    pthread_create(&request->tid, NULL, thread_send, request);
    return OSMP_SUCCESS;
}

int OSMP_Irecv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len, OSMP_Request request)  {
    request->buf = buf;
    request->count = count;
    request->datatype = datatype;
    request->source = source;
    request->len = len;

    pthread_create( &request->tid, NULL, thread_recv, request);
    return OSMP_SUCCESS;
}

int OSMP_Test(OSMP_Request request, int *flag){
    *flag= request->status;
    return OSMP_SUCCESS;
}

int OSMP_Wait(OSMP_Request request) {
    if(pthread_join(request->tid, NULL))
        return OSMP_ERROR;
    return OSMP_SUCCESS;
}






















