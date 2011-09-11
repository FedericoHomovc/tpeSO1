/***
***
***				transport.h
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
#define MSG_SIZE 1630

/***		Structs 		***/
typedef struct serverCDT * serverADT;
typedef struct clientCDT * clientADT;


typedef struct IPCMessage
{
	long size;
	void * message;
} message;

typedef struct
{
	pid_t id;
	clientADT client;
} infoClient;

/***		Functions		***/
serverADT createServer();

clientADT connectToServer(serverADT serv);

clientADT getClient(serverADT serv, pid_t id);

int sendMessage(clientADT client, message * msg, int flags);

int rcvMessage(clientADT client, message * msg, int flags);

int disconnectFromServer(clientADT client);

int terminateServer(serverADT server);

#endif
