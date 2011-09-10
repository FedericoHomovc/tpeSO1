/***
 ***
 ***		sockets.c
 ***				Jose Ignacio Galindo
 ***				Federico Homovc
 ***				Nicolas Loreti
 ***			 	     ITBA 2011
 ***
 ***/

/***		System includes		***/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <limits.h>
#include <signal.h>

/***		Project Includes		***/
#include "../../include/api.h"
#include "../../include/semaphore.h"

/***		Module Defines		***/
#define PATH_CONNECT "/tmp/socket_"
#define PATH_SIZE 20

#define SEM_CLI_TABLE 1
#define SIZE sizeof(struct sockaddr_un)
#define MAX_PATH_LENGTH 24


int public_sockfd;
void fatal(char *s);
/* Starts a server */
struct sockaddr_un * getStruct(int pid);

/*
 * Structs
 */

/* Struct underlying clientADT, have a sockFd Integer that represent the file
Socket descriptor */
struct clientCDT
{
	int sockfd;
	int semid;
	struct sockaddr_un data;
};

/* Struct underlying serverADT */
struct serverCDT
{
	/* Socket file descriptor for the server. */
	int sockFd;
	int semid;
	/* Socket address of the server. */
	struct sockaddr_in socketAddress;
};


serverADT startServer()
{
	int semid;

	serverADT server = malloc(sizeof(struct serverCDT));
	if (server == NULL)
		return NULL;

	if ( (server->sockFd = socket(AF_UNIX, SOCK_DGRAM,0) ) == -1)
	{
		perror("Socket call failed");
		free(server);
		return NULL;
	}

	semid = initSem(1);
	if (semid == -1) {
		free(server);
		fprintf(stderr, "Server: Semaphore initialization failed.");
		return NULL;
	}
	server->semid = semid;

	return server;	
}

/* Connects to a server */
clientADT connectToServer(serverADT server)
{
	struct sockaddr_un * addr_cli2;
	clientADT client;
	if( (client = malloc(sizeof(struct clientCDT))) == NULL)
		return NULL;
	
	down(server->semid, SEM_CLI_TABLE);

		addr_cli2 = getStruct(getpid());

		if ( (client->sockfd = socket(AF_UNIX, SOCK_DGRAM,0) ) == -1)
		{
			perror("Socket call failed");
			return NULL;
		}
		
		if ( bind(client->sockfd,(struct sockaddr *)addr_cli2, SIZE) ){
			perror("Son: bind call failed");
			return NULL;	
		}

		public_sockfd = client->sockfd;

	if (up(server->semid, SEM_CLI_TABLE, TRUE) == -1)
		return NULL;
	
	return client;
}

/* Gets a client from the client list */
clientADT getClient(serverADT server, pid_t id)
{

	struct sockaddr_un * addr;
	clientADT client = malloc(sizeof(struct clientCDT));
	if (client == NULL)
		return NULL;

	addr = getStruct(id);

	client->sockfd = public_sockfd;
	client->data = *addr;

	return client;
	
}

struct sockaddr_un * getStruct(int pid){
				
		char path1[MAX_PATH_LENGTH];
		struct sockaddr_un * addr_cli1 = malloc(SIZE);
    		
		sprintf(path1,"/tmp/sock_%d",pid);
		memset(addr_cli1, 0 , sizeof(struct sockaddr_un));
		addr_cli1->sun_family = AF_UNIX;
		memcpy(addr_cli1->sun_path, path1, sizeof(addr_cli1->sun_path)-1);

		return addr_cli1;
}

/* Sends a message */
int sendMsg(clientADT comm, message * msg, int flags)
{
	int ret;
	
	if( (ret = sendto(comm->sockfd,msg->message,msg->size,MSG_WAITALL,(struct sockaddr *)&comm->data, sizeof(struct sockaddr_un))) == -1){
		perror("server:sending");
		return -1;
	}

	return ret;
}

/* Receives a message */
int rcvMsg(clientADT comm, message *msg, int flags)
{
	int ret;

	if((ret = recvfrom(comm->sockfd,msg->message,msg->size,MSG_WAITALL,NULL,NULL)) == -1){
		perror("server: receiving");
		return -1;	
	}

	return ret;	
}

/* Disconnects a client from the server */
int disconnectFromServer(clientADT comm, serverADT server)
{
	printf("closing %s\n", comm->data.sun_path);
	unlink(comm->data.sun_path);
	free(comm);
	return 0;

}

/* Ends the server */
int endServer(serverADT server)
{
	close(server->sockFd);
	free(server);
	return 0;
}


void fatal(char *s) {
	perror(s);
	exit(1);
}
