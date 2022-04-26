//
// Created by studi on 19.04.22.
//

#ifndef OSMP_OSMPLIB_H
#define OSMP_OSMPLIB_H

#include "OSMP.h"

#define OSMP_SUCCESS 0
#define OSMP_ERROR -1

/**
 * Initializes the OSMP environment and enables access to the common Resources of the OSMP processes.
 * It must be called by every OSMP process at the beginning.
 * @param argc Address of the argument number.
 * @param argv Address of the argument vector.
 * @return OSMP_SUCCESS on success otherwise OSMP_ERROR.
 */
int OSMP_Init(const int *argc, char ***argv);

/**
 * This routine returns the number of OSMP processes without the OSMP starter process.
 * @param size an integer variable that will hold the total number of OSMP processes.
 * @return size on success otherwise OSMP_ERROR.
 */
int OSMP_Size(int *size);

/**
 * This routine returns the OSMP process ranking number of the calling OSMP process.
 * @param rank an integer variable that will hold the process ranking number.
 * @return rank on success otherwise OSMP_ERROR.
 */
int OSMP_Rank(int *rank);

/**
 * This routine sends a data to a process using process number as destination.
 * @param buf Start address of the buffer containing the data to be sent (input).
 * @param count Number of elements of the specified type in the buffer (input).
 * @param datatype OSMP-Type of the messages in the buffer (input).
 * @param dest recipient's number between 0,…,np-1 (input).
 * @return OSMP_SUCCESS on success otherwise OSMP_ERROR.
 */
int OSMP_Send(const void *buf, int count, OSMP_Datatype datatype, int dest);


/**
 * This routine allows The calling process to read a data with a maximum of count elements of the specified messages type.
 * @param buf Start address of the buffer in the local memory of the calling process in which the data is to be copied (input).
 * @param count maximum number of elements of the specified type that can be received (input).
 * @param datatype OSMP-Type of the messages in the buffer (input).
 * @param source sender's number between 0,…,np-1 (output).
 * @param len actual length of the received data in bytes (output).
 * @return OSMP_SUCCESS on success otherwise OSMP_ERROR.
 */
int OSMP_Recv(void *buf, int count, OSMP_Datatype datatype, int *source, int *len);

/**
 * All OSMP processes must call this routine before exiting. To munmap and shm_unlink the shared memory.
 * @return OSMP_SUCCESS on success otherwise OSMP_ERROR.
 */
int OSMP_Finalize(void);
/**
 * Kapselung der systemroutinen
 * @param sem der Semaphore zur Inkrementierung/dekrementierung
 */
void OSMP_wait(sem_t *sem);
void OSMP_signal(sem_t *sem);
#endif //OSMP_OSMPLIB_H
