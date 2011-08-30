/*
 * semaphore.h
 * Function prototypes for the implementation of a System V semaphore.
 * Authors:
 * Matías Colotto
 * Santiago Samra
 * Ezequiel Scaruli
 * Date: 14/03/2010
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

/*
 * Includes
 */

#include <sys/sem.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ipc.h>
#include <stdlib.h>

/*
 * Symbolic constants
 */

#define SEM_KEY (key_t)0x3FD
#define SEM_NUM 2
#define PERMFLAGS (0666 | IPC_CREAT | IPC_EXCL)

/*
 * semun
 * Description: This union is used to pass arguments to semctl
 * Fields:
 * - val: Initial value for the semaphore.
 * - buf: Unknown.
 * - array: Unknown.
 */
 
typedef union _semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
} semun;


/*
 * Name: initSem
 * Receives: void
 * Returns: int
 * Description: Initializes an array of SEM_NUM semaphores. 
 * In case of failure, prints an error message on stderr and
 * returns -1. Otherwise returns the semid of the new semarray.
 */
int initSem(void);

/*
 * Name: p
 * Receives: int semid, int semnum, int wait
 * Returns: int
 * Description: The classic wait semaphore command. Used to request 
 * exclusivity. Needs the semaphore id, the number in the array of 
 * semaphores (0 to SEM_NUM -1) and whether should it wait or not
 * in case the semaphore is taken (TRUE/FALSE). Returns 0 on success and -1 on
 * failure
 */
int p(int semid, int semnum, int wait);

/*
 * Name: v
 * Receives: int semid, int semnum
 * Returns: int
 * Description: The classic signal semaphore command. Used to leave
 * exclusivity. Needs the semaphore id and the number in the array of
 * semaphores (0 to SEM_NUM - 1). Returns 0 on success and -1 on
 * failure.
 */
int v(int semid, int semnum);

/*
 * Name: destroySem
 * Receives: int semid
 * Returns: int
 * Description: Function to end semaphores properly. Needs the semid 
 * to destroy the whole semarray. Returns 0 on success and -1 on failure.
 */
int destroySem(int semid);

#endif /* SEMAPHORE_H_*/
