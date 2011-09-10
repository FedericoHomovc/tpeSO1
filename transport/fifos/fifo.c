/***
***
***		fifo.c
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
#define	CLIENTS_SIZE	15
#define CHARS_ADD		10

#define SER_FIFO_S		"/tmp/serverFifo"
/*#define SER_FIFO_LEN	(strlen(SER_FIFO_S) + CHARS_ADD)*/
#define SER_FIFO_LEN	(25)

#define CLI_FIFO_R_S	"/tmp/clientFifo_r"
/*#define CLI_FIFO_R_LEN	(strlen(CLI_FIFO_R_S) + CHARS_ADD)*/
#define CLI_FIFO_R_LEN	(27)

#define CLI_FIFO_W_S	"/tmp/clientFifo_w"
/*#define CLI_FIFO_W_LEN	(strlen(CLI_FIFO_W_S) + CHARS_ADD)*/
#define CLI_FIFO_W_LEN	(27)

#define ERR_FIFO		-1

/*
 * Global variables.
 */

int serverConnected;


/*
 * Structs.
 */

/*
 * Name: struct clientCDT
 * Description: This struct is the implementation of clientADT for IPC via FIFOs.
 * Fields:
 * - clientFifo_r:	File descriptor for the FIFO used by a client to read.
 * - clientName:	Name of clientFifo_r in the file system.
 * - clientFifo_r:	File descriptor for the FIFO used by a client to write.
 * - clientName:	Name of clientFifo_w in the file system.
 */

struct clientCDT
{
	int clientFifo_r;
	char clientName_r[CLI_FIFO_R_LEN];

	int clientFifo_w;
	char clientName_w[CLI_FIFO_W_LEN];
};

/*
 * Name: struct serverCDT
 * Description: This struct is the implementation of serverADT for IPC via FIFOs.
 * Fields:
 * - serverFifo:	File descriptor for a FIFO through which the clients send
 * 					their information (struct connectionMsg) when they get
 * 					connected to the server.
 * - serverName:	Name of serverFifo in the file system.
 * - clients:		Array of clients which are currently connected to the
 * 					server.
 * - clientsUsed:	Number of clients in the array.
 */

struct serverCDT
{
	int serverFifo;
	char serverName[SER_FIFO_LEN];

	infoClient ** clients;
	int clientsUsed;
};


/*
 * Name: struct connectionMsg
 * Description: Message sent by clients to server when establishing a
 * connection.
 * Fields:
 * - id:			PID of the client establishing the connection.
 * - clientFifo_r:	File descriptor for the FIFO used by a client to read.
 * - clientName:	Name of clientFifo_r in the file system.
 * - clientFifo_r:	File descriptor for the FIFO used by a client to write.
 * - clientName:	Name of clientFifo_w in the file system.
 */
typedef struct
{
	pid_t id;

	int clientFifo_r;
	char clientName_r[CLI_FIFO_R_LEN];

	int clientFifo_w;
	char clientName_w[CLI_FIFO_W_LEN];

} connectionMsg;


static int mkopFifo(const char *expecName, char *resultName, mode_t modeCr,
			 mode_t modeOp)
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


static void *listeningFunction(void *serverInfo)
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
			/*
			 * An incoming client is found in the main server FIFO.
			 * It is stored into the client array.
			 */

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

			/*
			 * Pipes for reading and writing to the client are opened here.
			 * They are swapped in order to be treated indifferently by
			 * sndMsg and rcvMsg functions.
			 */
			if((fileDes_r = open(currentConnection.clientName_w, O_RDWR)) == -1)
			{
				fprintf(stderr, "Errno = %d, strerror = %s\n", errno, strerror(errno));
				fprintf(stderr,	"Client FIFO _w couldn't be opened at listening.\n");
				free(comm);
				return NULL;
			}

			comm->clientFifo_r = fileDes_r;

			
			if((fileDes_w = open(currentConnection.clientName_r, O_RDWR)) == -1)
			{
				fprintf(stderr, "Errno = %d, strerror = %s\n", errno, strerror(errno));
				fprintf(stderr,	"Client FIFO _r couldn't be opened at listening.\n");
				free(comm);
				return NULL;
			}

			comm->clientFifo_w = fileDes_w;

			strcpy(comm->clientName_r, currentConnection.clientName_r);
			strcpy(comm->clientName_w, currentConnection.clientName_w);
			currentClient->client = comm;
			currentClient->id = currentConnection.id;

			server->clients[server->clientsUsed] = currentClient;
			(server->clientsUsed)++;

			/* The listening thread sends a one-byte message to the client
			 * to ensure that the client was put into the array.
			 */

			msg.message = "OK";
			msg.size = 3;
			sendMsg(comm, &msg, 0);
		}
	}

	pthread_exit(NULL);

	return NULL;
}


/*
 * Main functions
 */


serverADT startServer(void)
{
	int fileDes, retValue;
	pthread_t listeningThread;
	serverADT ret;

	/* The return value is initialized. */

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

	/* Server listening thread is initialized. */
	if((retValue = pthread_create(&listeningThread, NULL, listeningFunction, ret)) == -1)
	{
		fprintf(stderr, "Listening thread couldn't be created. Error %d.\n", retValue);
		free(ret);
		return NULL;
	}

	pthread_detach(listeningThread);

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

	/* The two FIFOs are created. */
	if((fileDes_r = mkopFifo(CLI_FIFO_R_S, ret->clientName_r, 0666, O_RDWR)) == ERR_FIFO)
	{
		free(ret);
		return NULL;
	}
	ret->clientFifo_r = fileDes_r;
	
	if((fileDes_w = mkopFifo(CLI_FIFO_W_S, ret->clientName_w, 0666, O_RDWR)) == ERR_FIFO)
	{
		free(ret);
		return NULL;
	}
	ret->clientFifo_w = fileDes_w;

	/* Server's main FIFO is opened. */
	if((serverFileDes = open(serv->serverName, O_WRONLY)) == -1)
	{
		free(ret);
		fprintf(stderr, "Main FIFO couldn't be opened.\n");
		return NULL;
	}

	/* We have FIFOs for reading and writing. Now we have to send this
	 * info to the parent via the main FIFO. */
	mesg.id = getpid();
	mesg.clientFifo_r = ret->clientFifo_r;
	mesg.clientFifo_w = ret->clientFifo_w;
	strcpy(mesg.clientName_r, ret->clientName_r);
	strcpy(mesg.clientName_w, ret->clientName_w);

	if(write(serverFileDes, &mesg, sizeof(connectionMsg)) == -1)
	{
		fprintf(stderr, "The connection couldn't be established.\n");
		free(ret);
		return NULL;
	}

	/* The client waits a one-byte message from the server that indicates
	 * that it was added to the clients array.
	 */

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
	return write(comm->clientFifo_w, msg->message, msg->size);
}



int rcvMsg(clientADT comm, message *msg, int flags)
{
	if(comm == NULL || msg == NULL)
	{
		fprintf(stderr, "NULL parameters.\n");
		return -1;
	}
	return read(comm->clientFifo_r, msg->message, msg->size);
}


int disconnectFromServer(clientADT comm, serverADT server)
{
	if(comm == NULL || server == NULL)
		return -1;

	/* File descriptors are closed. */

	if(close(comm->clientFifo_r) == -1 || close(comm->clientFifo_w) == -1)
	{
		fprintf(stderr, "Client couldn't be closed.\n");
		return -1;
	}

	/* comm is freed. */

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

		close(currentComm->clientFifo_r);
		close(currentComm->clientFifo_w);

		if(unlink(currentComm->clientName_r) == -1 || unlink(currentComm->clientName_w) == -1)
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
