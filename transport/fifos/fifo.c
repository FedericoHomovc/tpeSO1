/***
***
***		fifo.c
***			Jose Ignacio Galindo
***			Federico Homovc
***			Nicolas Loreti
***		 	     ITBA 2011
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
#include "../../include/api.h"
#include "../../include/structs.h"
#include "../../include/backEnd.h"

/***		Module Defines		***/
#define CLI_FIFO_R_S	"/tmp/clientFifo_read"
#define CLI_FIFO_W_S	"/tmp/clientFifo_write"
#define SER_FIFO_S	"/tmp/serverFIFO"

#define	CLIENTS_SIZE	15
#define FIFO_NAMES	30
#define ERR_FIFO	-1


/***		Structs		***/

/*
 * struct clientCDT
 * This struct is the implementation of clientADT for IPC in FIFOs.
 * 
 * @clientFifo_read: File descriptor for the FIFO used by a client to read.
 * @clientName:	Name of clientFifo_read in the file system.
 * @clientFifo_read: File descriptor for the FIFO used by a client to write.
 * @clientName:	Name of clientFifo_write in the file system.
 */
struct clientCDT
{
	int clientFifo_read;
	char clientName_read[FIFO_NAMES];

	int clientFifo_write;
	char clientName_write[FIFO_NAMES];
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
 * @clientFifo_read: File descriptor for the FIFO used by a client to read.
 * @clientName: Name of clientFifo_read in the file system.
 * @clientFifo_read: File descriptor for the FIFO used by a client to write.
 * @clientName: Name of clientFifo_write in the file system.
*/

typedef struct
{
	pid_t id;

	int clientFifo_read;
	char clientName_read[FIFO_NAMES];

	int clientFifo_write;
	char clientName_write[FIFO_NAMES];

} connectionMsg;

/***		Functions		***/

/*
* function mkopFifo
*
* Creates a new FIFO file to communicate with. The file name and modes for creating
* and opening are given as parameters.
*
* @expecName: Constant, first part of fifo file name. Depends if fifo is server, 
* reading or writing.
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
			printf("Errno = %d.\n", errno);
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
	int fileDes_r, fileDes_w;
	serverADT server;
	connectionMsg currentConnection;
	clientADT comm;
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

			if( (comm = malloc(sizeof(struct clientCDT))) == NULL )
			{
				fprintf(stderr, "Not enough memory.\n");
				return NULL;
			}

			if((fileDes_r = open(currentConnection.clientName_write, O_RDWR)) == -1)
			{
				fprintf(stderr, "Errno = %d, strerror = %s\n", errno, strerror(errno));
				fprintf(stderr,	"Client FIFO_w couldn't be opened at listening.\n");
				free(comm);
				return NULL;
			}

			comm->clientFifo_read = fileDes_r;

			if((fileDes_w = open(currentConnection.clientName_read, O_RDWR)) == -1)

			{
				fprintf(stderr, "Errno = %d, strerror = %s\n", errno, strerror(errno));
				fprintf(stderr,	"Client FIFO_r couldn't be opened at listening.\n");
				free(comm);
				return NULL;
			}

			comm->clientFifo_write = fileDes_w;

			strcpy(comm->clientName_read, currentConnection.clientName_read);
			strcpy(comm->clientName_write, currentConnection.clientName_write);

			currentClient->client = comm;
			currentClient->id = currentConnection.id;

			server->clients[server->clientsUsed] = currentClient;
			(server->clientsUsed)++;

			msg.message = "OK"; 
			msg.size = 3;
			sendMsg(comm, &msg, 0); /*A message is sent to ensure that the client was created*/
		}
	}

	pthread_exit(NULL);

	return NULL;
}

serverADT startServer(void)
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
	int fileDes_r, fileDes_w, serverFileDes;
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

	if((fileDes_r = mkopFifo(CLI_FIFO_R_S, ret->clientName_read, 0666, O_RDWR)) == ERR_FIFO)
	{
		free(ret);
		return NULL;
	}
	ret->clientFifo_read = fileDes_r;
	
	if((fileDes_w = mkopFifo(CLI_FIFO_W_S, ret->clientName_write, 0666, O_RDWR)) == ERR_FIFO)
	{
		free(ret);
		return NULL;
	}
	ret->clientFifo_write = fileDes_w;

	if((serverFileDes = open(serv->serverName, O_WRONLY)) == -1)
	{
		free(ret);
		fprintf(stderr, "Server FIFO couldn't be opened.\n");
		return NULL;
	}

	mesg.id = getpid();
	mesg.clientFifo_read = ret->clientFifo_read;
	mesg.clientFifo_write = ret->clientFifo_write;
	strcpy(mesg.clientName_read, ret->clientName_read);
	strcpy(mesg.clientName_write, ret->clientName_write);

	if(write(serverFileDes, &mesg, sizeof(connectionMsg)) == -1)
	{
		fprintf(stderr, "The connection couldn't be established.\n");
		free(ret);
		return NULL;
	}

	if( (msg.message = calloc(3, sizeof(char))) == NULL)
		return NULL;
	msg.size = 3;

	if( rcvMsg(ret, &msg, 0) != -1 )
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


int sendMsg(clientADT comm, message *msg, int flags)
{
	if(comm == NULL || msg == NULL)
	{
		fprintf(stderr, "NULL parameters.\n");
		return -1;
	}
	return write(comm->clientFifo_write, msg->message, msg->size);
}



int rcvMsg(clientADT comm, message *msg, int flags)
{
	if(comm == NULL || msg == NULL)
	{
		fprintf(stderr, "NULL parameters.\n");
		return -1;
	}
	return read(comm->clientFifo_read, msg->message, msg->size);
}


int disconnectFromServer(clientADT comm, serverADT server)
{
	if(comm == NULL)
		return -1;

	if(close(comm->clientFifo_read) == -1 || close(comm->clientFifo_write) == -1)
	{
		fprintf(stderr, "Client couldn't be closed.\n");
		return -1;
	}
	free(comm);

	return 0;
}


int endServer(serverADT server)
{
	int i;
	clientADT currentComm;

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
		currentComm = server->clients[i]->client;
		close(currentComm->clientFifo_read);
		close(currentComm->clientFifo_write);

		if(unlink(currentComm->clientName_read) == -1 || unlink(currentComm->clientName_write) == -1)
		{
			fprintf(stderr, "Client FIFO couldn't be removed.\n");
			return -1;
		}

		free(currentComm);
		free(server->clients[i]);
	}

	free(server->clients);
	free(server);

	return 0;
}
