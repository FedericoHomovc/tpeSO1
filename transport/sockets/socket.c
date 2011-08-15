/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>
#include "../../include/api.h"
#include "../../include/varray.h"


/*
 * Constants
 */

/* Initial server listening port */
#define SERV_PORT_INIT 11000
/* Initial client connection port */
#define CLIENT_PORT_INIT 21000
/* Initial size for the infoClient vArray */
#define ICA_INITIAL_SIZE 100
/* Send a synchronization byte after connecting? */
#define SYNCHRO_BYTE_ON_CONNECT FALSE


/*
 * Structs
 */

/* Struct underlying comuADT */
struct IPCCDT
{
	/* Socket file descriptor to send/receive from. */
	int sockFd;
};

/* Struct underlying servADT */
struct serverCDT
{
	/* Socket address of the server. */
	struct sockaddr_in socketAddress;

	/* Socket file descriptor for the server. */
	int sockFd;

	vArray infoClientArray;
	pthread_t listeningThread;
};


/* Starts a server */
servADT startServer()
{
	servADT ret = malloc(sizeof(struct serverCDT));
	if (ret == NULL)
		return NULL;

	if ( (ret->sockFd = socket (AF_INET, SOCK_STREAM,0)) == -1)
	{
		free(ret);
		return NULL;
	}

	ret->socketAddress.sin_family = AF_INET;
	ret->socketAddress.sin_port = SERV_PORT_INIT;
	ret->socketAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	ret->infoClientArray = NULL;

	int bindRet = -1;
	while (bindRet == -1)
	{
		ret->socketAddress.sin_port++;
		bindRet = bind(ret->sockFd, (struct sockaddr *) &(ret->socketAddress),
				sizeof(struct sockaddr_in));
	}

	listen(ret->sockFd, 5);

	//pthread_create(&(ret->listeningThread), NULL, listeningConnection,
	//		(void *) ret);

	//pthread_detach(ret->listeningThread);

	return ret;
}

/* Connects to a server */
comuADT connectToServer(servADT serv)
{
	comuADT ret = malloc(sizeof(struct IPCCDT));
	if (ret == NULL)
		return NULL;

	ret->sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (ret->sockFd == -1)
	{
		free(ret);
		return NULL;
	}

	struct sockaddr_in socketAddress;
	socketAddress.sin_addr.s_addr = INADDR_ANY;
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = CLIENT_PORT_INIT;

	int bindRet = -1;
	while (bindRet == -1)
	{
		socketAddress.sin_port++;
		bindRet = bind(ret->sockFd, (struct sockaddr *) &socketAddress,
				sizeof(struct sockaddr_in));
	}

	int connectRet = -1;
	while (connectRet == -1)
		connectRet = connect(ret->sockFd,
				(struct sockaddr *) &(serv->socketAddress),
				sizeof(struct sockaddr_in));

	/* We are connected. Must send PID */
	pid_t pid;
	pid = getpid();
	message msg =
	{ sizeof(pid_t), &pid };
	while (sendMsg(ret, &msg, 0) < sizeof(pid_t))
		;

	/* We sent the PID to the server. Return */
	if (SYNCHRO_BYTE_ON_CONNECT)
	{
		/* But first, wait until the server sends me a byte */
		char c;
		message m =
		{ sizeof(char), &c };
		rcvMsg(ret, &m, 0);
	}
	return ret;
}

/* Gets a client from the client list */
comuADT getClient(servADT serv, pid_t id)
{
	if (serv == NULL || serv->infoClientArray == NULL)
		return NULL;
	infoClient ic;
	ic.id = id;
	ic.comm = NULL;
	infoClient * icp;

//= ((infoClient *) (vArray_search(serv->infoClientArray,
//			(int(*)(void *, void *)) infoClient_comparePid, &ic)));
	if (icp != NULL)
		return icp->comm;
	else
		return NULL;
}

/* Sends a message */
int sendMsg(comuADT comm, message * msg, int flags)
{
	int ret = 0;
printf("send-sockfd:%d\n",comm->sockFd);
	ret = send(comm->sockFd, msg->message, msg->size, 0);
	if ((flags & IPC_NOWAIT) == 0)
	{
		int countSent = ret;
		while (countSent < msg->size)
		{
			if (ret < 0)
			{
				fprintf(stderr, "Problem sending message.\n");
				return ret;
			}
			else
			{
				ret = send(comm->sockFd,
						&(((char *)(msg->message))[countSent]),
						msg->size - countSent, 0);
				countSent += ret;
			}
		}
		return countSent;
	}
	else
	{
		return ret;
	}
}

/* Receives a message */
int rcvMsg(comuADT comm, message *msg, int flags)
{
printf("send-sockfd:%d\n",comm->sockFd);
	int i = recv(comm->sockFd, msg->message, msg->size,
			(flags & IPC_NOWAIT) == 0 ? MSG_WAITALL : 0);
	return i;
}

/* Disconnects a client from the server */
int disconnectFromServer(comuADT comm, servADT server)
{
	close(comm->sockFd);
	free(comm);
	return 0;
}

/* Ends the server */
int endServer(servADT server)
{
	//pthread_cancel(server->listeningThread);
	close(server->sockFd);
	int i;
	for (i = 0; i < vArray_getSize(server->infoClientArray); i++)
	{
		infoClient * icp = vArray_getAt(server->infoClientArray, i);
		if (icp != NULL)
		{
			close(icp->comm->sockFd);
			free(icp->comm);
			free(icp);
		}
	}
	vArray_destroy(server->infoClientArray);
	free(server);
	return 0;
}

main(){

printf("Esto funciona..\n\n");

if( fork() == 0){

servADT server = startServer();
printf("Creo el server\n");
}else{
comuADT cliente1 = connectToServer(server);
printf("Cliente 1 enganchado\n");

comuADT cliente2 = connectToServer(server);
printf("Cliente 2 enganchado\n\n");

message mensaje = {10,"holaholac"};
message * msg = &mensaje;
message * rcvmsg;
//printf("Longitud:%ld\nMensaje:%s\n",rcvmsg->size,(char *)rcvmsg->message);

printf("\nMandando...\n");
printf("Longitud:%ld\nMensaje:%s\n",msg->size,(char *)msg->message);
sendMsg(cliente1,msg,0);
printf("Mensaje Mandado!\n\n");

while( rcvMsg(cliente2,rcvmsg,0) > 0){
sendMsg(cliente1,msg,0);
}

printf("Recibiendo mensaje...\n");
printf("Mensaje recivido\nEl mensaje es:\n");
printf("Longitud:%ld\nMensaje:%s\n",rcvmsg->size,(char *)rcvmsg->message);

//endServer(server);
//printf("Hizo pelota el server\n");

printf("Esto termino\n");
}
}
