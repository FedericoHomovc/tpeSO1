/***
 ***
 ***		semaphore.c
 ***		Jose Ignacio Galindo
 ***		Federico Homovc
 ***		Nicolas Loreti
 ***		ITBA 2011
 ***
 ***/

#include <string.h>
#include "../include/semaphore.h"

/*
 * name: initSem
 * description: initializes a semaphore array of length 2.
 * @value:it represents the initial value of both semaphores.
 *
 */
int initSem(int value)
{
    int semid, status = 0, status2 = 0;
    semun arg;
    int i = 0;
    
    while( (semid = semget( SEM_KEY + i, SEM_NUM, FLAGS)) == -1)
        i++;
    
    /* Set recently created semaphores to value 1 */
    arg.val = value;
    status = semctl(semid, 0, SETVAL, arg);
    status2 = semctl(semid, 1, SETVAL, arg);
 
    
    if( status == -1 || status2 == -1 || semid == -1)
    {
        fprintf(stderr, "initSem(): Semaphore initialization failed\n");
        return -1;
    }
    return semid;
    
}

/*
 * name:up (AKA p)
 * description: Semaphore wait operation
 * @semid: unique identification of the semaphore.
 * @semnum: number of the semaphore wished to be used
 * @wait: whether the semaphore has to wait or not
 */
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
            fprintf(stderr, "up(): The semaphore is red. Must wait\n");
        else
            fprintf(stderr, "up(): Semaphore up operation failed. Semid: %d\n", semid);
        return -1;
    }
    return 0;
}

/*
 * name:down (AKA v)
 * description: Semaphore signal operation.
 * @semid: unique identification of the semaphore.
 * @semnum: number of the semaphore wished to be used
 */
int down(int semid, int semnum)
{
    struct sembuf buffer;
    buffer.sem_num = semnum;
    buffer.sem_op = 1;
    buffer.sem_flg = SEM_UNDO;
    
    if( semop(semid, &buffer, (size_t) 1) == -1)
    {
        fprintf(stderr, "down(): Semaphore down operation failed. Semid: %d\n", semid);
        return -1;
    }
    return 0;
}

/*
 * name:destroySem
 * description: Destroy the semaphore array.
 * @semid: unique identification of the semaphore.
 */
int destroySem(int semid)
{
    int ans = semctl(semid, 0, IPC_RMID, NULL);
    if( ans == -1)
    {
        fprintf(stderr, "destroySem(): The semaphore %d couldn't be destroyed\n", semid);
    }
    return ans;
}
