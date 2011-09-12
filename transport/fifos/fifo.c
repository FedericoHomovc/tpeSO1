/***
***
***			fifo.c
***			Jose Ignacio Galindo
***			Federico Homovc
***			Nicolas Loreti
***		 	ITBA 2011
***
***/


/***		System includes		***/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>

/***		Project Includes		***/
#include "../../include/transport.h"
#include "../../include/structs.h"
#include "../../include/backEnd.h"

/***		Module Defines		***/
#define CLI_FIFO	"/tmp/clientFifo"
#define SER_FIFO_S	"/tmp/serverFIFO"

#define	CLIENTS_SIZE	15
#define FIFO_NAMES	30
#define ERR_FIFO	-1


/***		Structs		***/

/*
 * struct clientCDT
 * This struct is the implementation of clientADT for IPC in FIFOs.
 * 
 * @clientFifo: File descriptor for the FIFO used by a client to read and write.
 * @clientName:	Name of client Fifo in the file system.
 */
struct clientCDT
{
	int clientFifo;
	char clientName[FIFO_NAMES];
};

/*
 * struct serverCDT
 * This struct is the implementation of serverADT for IPC in FIFOs.
 * 
 * @serverFifo: File descriptor for a FIFO through which the clients send their 
 * 	information (struct connectionMsg) when they get connected to the server.
 * @serverName: Name of serverFifo in the file system.
 * @clients: Array of clients which are currently connected to the server.
 * @clientsUsed: Number of clients in the array.
 */
struct serverCDT
{
	int serverFifo;
	char serverName[FIFO_NAMES];

	infoClient ** clients;
	int clientsUsed;
};


/*
 * struct connectionMsg
 * Message sent by clients to server when establishing a connection.
 *
 * @id: PID of the client establishing the connection.
 * @clientFifo: File descriptor for the FIFO used by a client to read and write.
 * @clientName: Name of client Fifo in the file system.
*/

typedef struct
{
	pid_t id;

	int clientFifo;
	char clientName[FIFO_NAMES];

} connectionMsg;

/***		Functions		***/

/*
* function mkopFifo
*
* Creates a new FIFO file to communicate with. The file name and modes for creating
* and opening are given as parameters.
*
* @expecName: Constant, first part of fifo file name. Depends if fifo is server, 
* or client.
* @resulName: Final name of the fifo file. Includes the first part and a number that 
* differentiates each file.
* @modeCr: Mode for creating fifo.
* @modeOp: Mode for opening fifo.
*/
static int mkopFifo(const char *expecName, char *resultName, mode_t modeCr, mode_t modeOp);

/*
* function listeningThread
*
* Thread that is constantly checking if a new client connects to server. In such a case,
* reads from the Server Fifo file and creates a new client with the specified data.
*
* @serverInfo: Void pointer to the current server information.
*/
static void * listeningThread(void *serverInfo);

int serverConnected;

static int mkopFifo(const char *expecName, char *resultName, mode_t modeCr, mode_t modeOp)
{
	int count = 0, ret, length;
	strcpy(resultName, expecName);
	length = strlen(resultName);

	while(mkfifo(resultName, modeCr) == -1)
	{
		if(errno != EEXIST)
		{
			fprintf(stderr, "FIFO couldn't be created.\n");
			return ERR_FIFO;
		}

		itoa(count, resultName + length);
		count++;
	}

	if((ret = open(resultName, modeOp)) == -1)
	{
		fprintf(stderr, "FIFO couldn't be opened.\n");
		return ERR_FIFO;
	}

	return ret;
}


static void * listeningThread(void *serverInfo)
{
	int fileDes;
	serverADT server;
	connectionMsg currentConnection;
	clientADT client;
	infoClient * currentClient;
	message msg;

	server = (serverADT)serverInfo;
	server->clients = malloc( sizeof(infoClient*) * CLIENTS_SIZE );

	while(serverConnected)
	{
		if(read(server->serverFifo, &currentConnection, sizeof(connectionMsg)) != -1)
		{
			if( (currentClient = malloc(sizeof(infoClient))) == NULL)
			{
				fprintf(stderr, "Not enough memory.\n");
				return NULL;
			}

			if( (client = malloc(sizeof(struct clientCDT))) == NULL )
			{
				fprintf(stderr, "Not enough memory.\n");
				return NULL;
			}

			if((fileDes = open(currentConnection.clientName, O_RDWR)) == -1)
			{
				fprintf(stderr, "Errno = %d, strerror = %s\n", errno, strerror(errno));
				fprintf(stderr,	"Client FIFO couldn't be opened.\n");
				free(client);
				return NULL;
			}
			client->clientFifo = fileDes;

			strcpy(client->clientName, currentConnection.clientName);
			currentClient->client = client;
			currentClient->id = currentConnection.id;

			server->clients[server->clientsUsed] = currentClient;
			(server->clientsUsed)++;

			msg.message = "OK"; 
			msg.size = 3;
			sendMessage(client, &msg, 0); /*A message is sent to ensure that the client was created*/
		}
	}

	pthread_exit(NULL);

	return NULL;
}

serverADT createServer(void)
{
	int fileDes, retValue;
	pthread_t thread;
	serverADT ret;

	if( (ret = malloc(sizeof(struct serverCDT))) == NULL)
		return NULL;

	if((fileDes = mkopFifo(SER_FIFO_S, ret->serverName, 0666, O_RDWR | O_NONBLOCK)) == ERR_FIFO)
	{
		free(ret);
		return NULL;
	}

	ret->serverFifo = fileDes;
	ret->clients = NULL;
	ret->clientsUsed = 0;
	serverConnected = TRUE;

	if((retValue = pthread_create(&thread, NULL, listeningThread, ret)) == -1)
	{
		fprintf(stderr, "Listening thread couldn't be created. Error %d.\n", retValue);
		free(ret);
		return NULL;
	}

	pthread_detach(thread);

	return ret;
}


clientADT connectToServer(serverADT serv)
{
	int fileDes, serverFileDes;
	connectionMsg mesg;
	clientADT ret;
	message msg;

	if(serv == NULL)
	{
		fprintf(stderr, "Server must not be NULL");
		return NULL;
	}

	if( (ret = malloc(sizeof(struct clientCDT))) == NULL)
		return NULL;

	if((fileDes = mkopFifo(CLI_FIFO, ret->clientName, 0666, O_RDWR)) == ERR_FIFO)
	{
		free(ret);
		return NULL;
	}
	ret->clientFifo = fileDes;

	if((serverFileDes = open(serv->serverName, O_WRONLY)) == -1)
	{
		free(ret);
		fprintf(stderr, "Server FIFO couldn't be opened.\n");
		return NULL;
	}

	mesg.id = getpid();
	mesg.clientFifo = ret->clientFifo;
	strcpy(mesg.clientName, ret->clientName);

	if(write(serverFileDes, &mesg, sizeof(connectionMsg)) == -1)
	{
		fprintf(stderr, "The connection couldn't be established.\n");
		free(ret);
		return NULL;
	}

	if( (msg.message = calloc(3, sizeof(char))) == NULL)
		return NULL;
	msg.size = 3;

	if( rcvMessage(ret, &msg, 0) != -1 )
		if( strcmp((char*)msg.message, "OK") )
			return NULL;

	free(msg.message);

	return ret;
}


clientADT getClient(serverADT serv, pid_t id)
{
	int i;
	
	for(i = 0; i < serv->clientsUsed; i++)
		if( !(serv->clients[i]->id - id) )
			return serv->clients[i]->client;

	return NULL;
}


int sendMessage(clientADT client, message *msg, int flags)
{
	if( client == NULL || msg == NULL)
	{
		fprintf(stderr, "NULL parameters.\n");
		return -1;
	}
	return write(client->clientFifo, msg->message, msg->size);
}



int rcvMessage(clientADT client, message *msg, int flags)
{
	if(client == NULL || msg == NULL)
	{
		fprintf(stderr, "NULL parameters.\n");
		return -1;
	}
	return read(client->clientFifo, msg->message, msg->size);
}


int disconnectFromServer(clientADT client)
{
	if(client == NULL)
		return -1;

	free(client);

	return 0;
}


int terminateServer(serverADT server)
{
	int i;
	clientADT currentclient;

	if(server == NULL)
		return -1;

	serverConnected = FALSE;

	close(server->serverFifo);

	if(unlink(server->serverName) == -1)
	{
		fprintf(stderr, "Server FIFO couldn't be removed.\n");
		return -1;
	}

	for(i = 0; i < server->clientsUsed; i++)
	{
		currentclient = server->clients[i]->client;
		close(currentclient->clientFifo);

		if(unlink(currentclient->clientName) == -1)
		{
			fprintf(stderr, "Client FIFO couldn't be removed.\n");
			return -1;
		}

		free(currentclient);
		free(server->clients[i]);
	}

	free(server->clients);
	free(server);

	return 0;
}
