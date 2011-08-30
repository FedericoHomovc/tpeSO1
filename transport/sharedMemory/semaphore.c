/*
 * semaphore.c
 * Implements a semaphore for shm.c needs.
 * Authors:
 * Mat�as Colotto
 * Santiago Samra
 * Ezequiel Scaruli
 * Date: 14/03/2010
 */

#include "../../include/semaphore.h"
#include <string.h>

/* Initializes a semaphore array. */
int initSem(void)
{
    int semid, status = 0, status2 = 0;
    semun arg;
    int i = 0;
    
    while( (semid = semget( SEM_KEY + i, SEM_NUM, PERMFLAGS)) == -1)
        i++;
    
    /* Set recently created semaphores to value 1 */
    arg.val = 1;
    status = semctl(semid, 0, SETVAL, arg);
    status2 = semctl(semid, 1, SETVAL, arg);
 
    
    if( status == -1 || status2 == -1 || semid == -1)
    {
        fprintf(stderr, "Semaphore initialization failed\n");
        return -1;
    }
    return semid;
    
}

/* Semaphore wait operation */
int p(int semid, int semnum, int wait)
{
    struct sembuf buffer;
    buffer.sem_num = semnum;
    buffer.sem_op = -1;
    if( wait )
        buffer.sem_flg = SEM_UNDO;
    else
        buffer.sem_flg = SEM_UNDO | IPC_NOWAIT;
    
    if( semop(semid, &buffer, (size_t) 1) == -1)
    {
        if( errno == EAGAIN )
        {
            fprintf(stderr, "The semaphore is taken. Please wait\n");
        }
        else
        {
            fprintf(stderr, "Semaphore P operation failed. Semid: %d\n", semid);
            fprintf(stderr, "Errno is %d: %s\n", errno, strerror(errno));
        }
        return -1;
    }
    return 0;
}

/* Semaphore signal operation */
int v(int semid, int semnum)
{
    struct sembuf buffer;
    buffer.sem_num = semnum;
    buffer.sem_op = 1;
    buffer.sem_flg = SEM_UNDO;
    
    if( semop(semid, &buffer, (size_t) 1) == -1)
    {
        fprintf(stderr, "Semaphore V operation failed. Semid: %d\n", semid);
        fprintf(stderr, "Errno is %d: %s\n", errno, strerror(errno));
        return -1;
    }
    return 0;
}

/* Destroy the array of semaphores */
int destroySem(int semid)
{
    int result = semctl(semid, 0, IPC_RMID, NULL);
    if( result == -1)
    {
        fprintf(stderr, "The semaphore %d couldn't be destroyed\n", semid);
    }
    return result;
}
