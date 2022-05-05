//
// Created by studi on 19.04.22.
//

#ifndef OSMP_OSMP_H
#define OSMP_OSMP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>

#define OSMP_MAX_MESSAGES_PROC   16 // maximale Zahl der Nachrichten pro Prozess
#define OSMP_MAX_SLOTS           256 // maximale Anzahl der Nachrichten, die insgesamt vorhanden sein dürfen
#define OSMP_MAX_PAYLOAD_LENGTH  1024 // maximale Länge der Nutzlast einer Nachricht
#define OSMP_MAX_PROCESSES  100 // maximale Zahl der Prozesses
#define OSMP_BCAST_SLOT 255 // Slot an dem Broadcast Nachrichten liegen

#define FORKERR 10
#define EXECERR 11
#define WAITERR 12
#define SHMOPENERR 13
#define FTRUNCERR 14
#define MMAPERR 15
#define UNLINKERR 16
#define MUNMAPERR 17
#define SEMERR 18
#define PTHREAD_BARRIER_ERROR 19

#define OSMP_SHM_NAME "OSMP_sh_mem"
#define ERROR_ROUTINE(code) fprintf(stderr, "%s\n", strerror(errno)); \
                                    exit(code);
typedef void* OSMP_Request;
//Aufzählung für den Datentyp
typedef enum {
    osmp_short,
    osmp_int,
    osmp_long,
    osmp_unsigned_char = sizeof(unsigned char),
    osmp_unsigned_short,
    osmp_unsigned,
    osmp_unsigned_long,
    osmp_float,
    osmp_double,
    osmp_byte,
} OSMP_Datatype;

struct barrier{
    volatile int count;
    sem_t sem_barrier;
};

typedef struct{
    pthread_mutex_t bcast_mutex;
    pthread_cond_t bcast_cond;
    volatile int cond;
} barrier_t;
struct parameters {
    int size;
    struct shared_memory *shm_pointer; //int ? struct shared_memory* no?
    int rank;
};

struct message {
    int sender_pr_rank;
    int elt_zahl;
    int msg_len;
    OSMP_Datatype elt_datentyp;
    int next_free_msg_slot;
    char payload[OSMP_MAX_PAYLOAD_LENGTH];
};

struct process {
    int rank;
    pid_t pid;
    int read_index;
    int write_index;
    sem_t proc_mutex;
    sem_t belegte_slots;
    sem_t freie_slots;
    int msg_slots[OSMP_MAX_MESSAGES_PROC];
};

struct shared_memory {
    sem_t shm_mutex;
    sem_t free_slots;
    sem_t belegte_slots;

    //struct barrier barrier; //-> Broadcast mit Semaphoren
    //pthread_barrier_t barrier; // -> Broadcast mit pthread_barrier
    barrier_t cond_barrier; //-> broadcast mit condition Variables
    off_t shm_size;
    int stack_index;
    int actual_free_slot;
    int size;
    struct message messages[OSMP_MAX_SLOTS];
    struct process processes[OSMP_MAX_PROCESSES];
};

#endif //OSMP_OSMP_H
