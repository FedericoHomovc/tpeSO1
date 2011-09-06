/***
***
***		api.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

#ifndef API_H_
#define API_H_

/***		System includes		***/
#include <sys/types.h>


/***		Module Defines		***/
#define TRUE 1
#define FALSE 0
#define MSG_SIZE 3000

/***		Structs 		***/
typedef struct serverCDT * servADT;
typedef struct clientCDT * comuADT;


typedef struct IPCMessage
{
	long size;
	void * message;
} message;

typedef struct
{
	pid_t id;
	comuADT comm;
} infoClient;

typedef struct{
	pid_t pid;
	servADT server;
}processData;

/***		Functions		***/
servADT startServer();

comuADT connectToServer(servADT serv);

comuADT getClient(servADT serv, pid_t id);

int sendMsg(comuADT comm, message * msg, int flags);

int rcvMsg(comuADT comm, message * msg, int flags);

int disconnectFromServer(comuADT comm, servADT server);

int endServer(servADT server);

int infoClient_comparePid(infoClient * ic1, infoClient * ic2);

#endif
