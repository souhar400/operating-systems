//
// Created by studi on 19.04.22.
//

#ifndef OSMP_DATASTRUCTS_H
#define OSMP_DATASTRUCTS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define OSMP_MAX_MESSAGES_PROC   16 // maximale Zahl der Nachrichten pro Prozess
#define OSMP_MAX_SLOTS           256 // maximale Anzahl der Nachrichten, die insgesamt vorhanden sein dürfen
#define OSMP_MAX_PAYLOAD_LENGTH  1024 // maximale Länge der Nutzlast einer Nachricht
#define OSMP_MAX_PROCESSES  100 // maximale Zahl der Prozesses

//Aufzählung für den Datentyp
typedef enum {
    osmp_short,
    osmp_int,
    osmp_long,
    osmp_unsigned_char,
    osmp_unsigned_short,
    osmp_unsigned,
    osmp_unsigned_long,
    osmp_float,
    osmp_double,
    osmp_byte,
} OSMP_Datatype;

struct parameters {
    int size;
    int *shm_pointer;
    int rank;
};

struct message {
    int sender_pr_rank;
    int receiver_pr_rank;
    int elt_zahl;
    int msg_len;
    OSMP_Datatype elt_datentyp;
};

struct process {
    int pid;
    int rank;
    int messages_zahl;
};

struct shared_memory {
    off_t shm_size;
    int available_messages;
    int size;
    struct process processes[OSMP_MAX_PROCESSES];
    struct message messages[OSMP_MAX_SLOTS];
};

#endif //OSMP_DATASTRUCTS_H
