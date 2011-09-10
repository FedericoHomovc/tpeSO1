/***
 ***
 ***		msgQueue.c
 ***				Jose Ignacio Galindo
 ***				Federico Homovc
 ***				Nicolas Loreti
 ***			 	     ITBA 2011
 ***
 ***/

/***		System includes		***/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

/***		Project Includes		***/
#include "../../include/api.h"
#include "../../include/structs.h"

/***		Structs 		***/
typedef struct {
	long mtype;
	char mtext[MSG_SIZE];
} msgQueue;


struct serverCDT {
	int queueID;
};

struct clientCDT {
	int queueID;
	int pid;
};



static key_t queueKey = 0xBEEF0;
int queueid;

serverADT startServer() {
	serverADT serv;

	if ((queueid = msgget(queueKey, 0666 | IPC_CREAT)) == -1)
	{
		printf("Error msgget\n");
		return NULL;
	}

	if( (serv = malloc(sizeof(struct serverCDT))) == NULL )
		return NULL;
	serv->queueID = queueid;
	return serv;
}

clientADT connectToServer(serverADT serv) {
	clientADT client;
	if( (client = malloc( sizeof(struct clientCDT))) == NULL)
		return NULL;
	client->queueID = serv->queueID;
	client->pid = getpid();

	return client;
}

clientADT getClient(serverADT serv, pid_t id) {
	clientADT client = malloc( sizeof(clientADT) );
	if( (client = malloc( sizeof(struct clientCDT))) == NULL)
		return NULL;
	client->queueID = serv->queueID;
	client->pid = id;
	return client;
}

int sendMsg(clientADT client, message * msg, int flags) {
	msgQueue aux;
	aux.mtype = client->pid;
	strcpy(aux.mtext, msg->message);
	
	return msgsnd(client->queueID, &aux,sizeof aux.mtext, flags);
}

int rcvMsg(clientADT client, message * msg, int flags) {
	int ret;
	msgQueue aux;
	ret = msgrcv(client->queueID, &aux, sizeof aux.mtext, client->pid, flags);
	strcpy(msg->message, aux.mtext);
	msg->size = strlen(msg->message);
	return ret;
}

int disconnectFromServer(clientADT client, serverADT server)
{
	free(client);
	return 0;
}

int endServer(serverADT server)
{
	msgctl(queueid, IPC_RMID, NULL);
	free(server);
	return 0;
}
