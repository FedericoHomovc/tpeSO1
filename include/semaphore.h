/***
***
***		semaphore.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

/***		System includes		***/
#include <sys/sem.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ipc.h>
#include <stdlib.h>

/***		Module Defines		***/
#define SEM_KEY (key_t)0x3FD
#define SEM_NUM 2
#define FLAGS (0666 | IPC_CREAT | IPC_EXCL)

 
typedef union _semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
} semun;



int initSem(int value);


int up(int semid, int semnum, int wait);


int down(int semid, int semnum);


int destroySem(int semid);

#endif
