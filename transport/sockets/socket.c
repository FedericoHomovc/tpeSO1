/***
 ***
 ***		sockets.c
 ***		Jose Ignacio Galindo
 ***		Federico Homovc
 ***		Nicolas Loreti
 ***   	    ITBA 2011
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
#include "../../include/transport.h"
#include "../../include/semaphore.h"

/***		Module Defines		***/
#define PATH_CONNECT "/tmp/socket_"
#define PATH_SIZE 20
#define SEM_CLI_TABLE 1
#define SIZE sizeof(struct sockaddr_un)
#define MAX_PATH_LENGTH 24


int public_sockfd;

/* Starts a server */
struct sockaddr_un * getStruct(int pid);

/*
 * Structs
 */

/* Struct underlying clientADT, have a sockFd Integer that represent the file
Socket descriptor, a integer semid that represents a semaphore's ID, a filePath
and the struct necessary to connect with other processes */

struct clientCDT
{
	int sockfd;
	int semid;
	char filePath[MAX_PATH_LENGTH];
	struct sockaddr_un data;
};

/* Struct underlying serverADT, have a sockFd integer that represent the file
Socket descriptor, a semid integer for a semaphora and the struct with the 
server information */
struct serverCDT
{
	/* Socket file descriptor for the server. */
	int sockFd;
	int semid;
	/* Socket address of the server. */
	struct sockaddr_in socketAddress;
};

/* CreateServer()
 *
 * Creates a serverADT struct and a semaphore. The implementation use UDP 
 * instead of TCP. 
 * 
 */

serverADT createServer()
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


/* ConnectToServer()
 *
 * The function creates a ClientADT. The implementation is base on AF_UNIX and
 * UDP. And the path style is "/tmp/socket_PID" where PID is the process id.
 * 
 */
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
		client->data = *addr_cli2;

	if (up(server->semid, SEM_CLI_TABLE, TRUE) == -1)
		return NULL;
	
	return client;
}

/* getClient()
 *
 * Creates and return clientADT based on the information of the integer 
 * parameter "id" which represents the process id from the process I want to 
 * connect to. 
 *
 */
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

/* getStruct()
 *
 * Recieves a process id and return a complete sockaddr_un struct 
 * with the family and the path to make a possible client.
 *
*/
struct sockaddr_un * getStruct(int pid){
				
		char path1[MAX_PATH_LENGTH];
		struct sockaddr_un * addr_cli1 = malloc(SIZE);
    		
		sprintf(path1,"/tmp/sock_%d",pid);
		memset(addr_cli1, 0 , sizeof(struct sockaddr_un));
		addr_cli1->sun_family = AF_UNIX;
		memcpy(addr_cli1->sun_path, path1, sizeof(addr_cli1->sun_path)-1);

		return addr_cli1;
}

/* sendMessage()
 *
 * Send a message to a client.
 *
 */

int sendMessage(clientADT client, message * msg, int flags)
{
	int ret;
	
	if((ret=sendto(client->sockfd,msg->message,msg->size,MSG_WAITALL,(struct sockaddr *)&client->data, sizeof(struct sockaddr_un))) == -1)
	{
		perror("server:sending");
		return -1;
	}

	return ret;
}


/* sendMessage()
 *
 * Receive a message from a client
 *
 */

int rcvMessage(clientADT client, message *msg, int flags)
{
	int ret;

	if((ret = recvfrom(client->sockfd,msg->message,msg->size,MSG_WAITALL,NULL,NULL)) == -1){
		perror("server: receiving");
		return -1;	
	}

	return ret;	
}

/* disconnectFromServer()
 *
 * Disconnects a client.
 *
 */
int disconnectFromServer(clientADT client)
{
	unlink(client->data.sun_path);
	free(client);
	return 0;

}

/* terminateServer()
 *
 * Ends a server.
 *
 */

int terminateServer(serverADT server)
{
	close(server->sockFd);
	free(server);
	return 0;
}


