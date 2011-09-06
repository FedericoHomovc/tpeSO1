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

typedef struct{
	pid_t pid;
	serverADT server;
}processData;

/***		Functions		***/
serverADT startServer();

clientADT connectToServer(serverADT serv);

clientADT getClient(serverADT serv, pid_t id);

int sendMsg(clientADT client, message * msg, int flags);

int rcvMsg(clientADT client, message * msg, int flags);

int disconnectFromServer(clientADT client, serverADT server);

int endServer(serverADT server);

int infoClient_comparePid(infoClient * ic1, infoClient * ic2);

#endif
