/*
 * semaphore.c
 * Implements a semaphore for shm.c needs.
 * Authors:
 *
 *
 */

#include "../../include/semaphore.h"
#include <string.h>

/* Initializes a semaphore array. */
int initSem(void)
{
    int semid, status = 0, status2 = 0;
    semun arg;
    int i = 0;
    
    while( (semid = semget( SEM_KEY + i, SEM_NUM, FLAGS)) == -1)
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
int up(int semid, int semnum, int wait)
{
    struct sembuf buffer;
    buffer.sem_num = semnum;
    buffer.sem_op = -1;


    buffer.sem_flg = SEM_UNDO;
    if( !wait )
        buffer.sem_flg = SEM_UNDO | IPC_NOWAIT;
    
    if( semop(semid, &buffer, (size_t) 1) == -1)
    {
        if( errno == EAGAIN )
        {
            fprintf(stderr, "The semaphore is red. Must wait\n");
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
int down(int semid, int semnum)
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
