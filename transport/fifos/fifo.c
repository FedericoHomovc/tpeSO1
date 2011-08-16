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
#include "../../include/varray.h"
#include "../../include/fifo.h"


/*
 * Global variables.
 */

int serverConnected;


/*
 * Structs.
 */

/*
 * Name: struct serverCDT
 * Description: This struct is the implementation of servADT for IPC via FIFOs.
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

	vArray clients;
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
	int count = 0;
	int length = strlen(resultName);
	int ret;

	strcpy(resultName, expecName);

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
	servADT server = (servADT)serverInfo;
	comuADT comm;
	connectionMsg currentConnection;
	int fileDes_r;
	int fileDes_w;
	char c;
	message msg;

	server->clients = vArray_init(CLIENTS_SIZE);

	while(serverConnected)
	{
		if(read(server->serverFifo, &currentConnection, sizeof(connectionMsg))
				!= -1)
		{
			/*
			 * An incoming client is found in the main server FIFO.
			 * It is stored into the client array.
			 */

			infoClient *currentClient = malloc(sizeof(infoClient));

			if(currentClient == NULL)
			{
				fprintf(stderr, "Not enough memory.\n");
				return NULL;
			}

			comm = malloc(sizeof(struct IPCCDT));

			if(comm == NULL)
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
				fprintf(stderr, "Errno = %d, strerror = %s\n", errno,
						strerror(errno));
				fprintf(stderr,
						"Client FIFO _w couldn't be opened at listening.\n");
				free(comm);
				return NULL;
			}

			comm->clientFifo_r = fileDes_r;

			

			if((fileDes_w = open(currentConnection.clientName_r, O_RDWR)) == -1)
			{
				fprintf(stderr, "Errno = %d, strerror = %s\n", errno,
						strerror(errno));
				fprintf(stderr,
						"Client FIFO _r couldn't be opened at listening.\n");
				free(comm);
				return NULL;
			}

			comm->clientFifo_w = fileDes_w;

			strcpy(comm->clientName_r, currentConnection.clientName_r);
			strcpy(comm->clientName_w, currentConnection.clientName_w);

			currentClient->comm = comm;
			currentClient->id = currentConnection.id;

			vArray_insertAtEnd(server->clients, currentClient);
			(server->clientsUsed)++;

			/* The listening thread sends a one-byte message to the client
			 * to ensure that the client was put into the array.
			 */

			
			msg.size= 1;
			msg.message = &c;
			sendMsg(comm, &msg, 0);
		}
	}

	pthread_exit(NULL);

	return NULL;
}


/*
 * Main functions
 */


servADT startServer(void)
{
	pthread_t listeningThread;
	int retValue;
	int fileDes;
	servADT ret = malloc(sizeof(struct serverCDT));

	/* The return value is initialized. */

	if(ret == NULL)
		return NULL;


	if((fileDes = mkopFifo(SER_FIFO_S, ret->serverName, 0666,
			O_RDWR | O_NONBLOCK)) == ERR_FIFO)
	{
		free(ret);
		return NULL;
	}

	ret->serverFifo = fileDes;

	ret->clients = NULL;

	ret->clientsUsed = 0;

	serverConnected = TRUE;

	/* Server listening thread is initialized. */


	if((retValue = pthread_create(&listeningThread, NULL, listeningFunction,
			ret)) == -1)
	{
		fprintf(stderr, "Listening thread couldn't be created. Error %d.\n",
				retValue);
		free(ret);
		return NULL;
	}

	pthread_detach(listeningThread);

	return ret;
}


comuADT connectToServer(servADT serv)
{
	int fileDes_r;
	int fileDes_w;
	int serverFileDes;
	char c;
	pid_t id;
	connectionMsg mesg;
	message msg;
	comuADT ret = malloc(sizeof(struct IPCCDT));

	if(serv == NULL)
	{
		fprintf(stderr, "Server must not be NULL");
		return NULL;
	}

	if(ret == NULL)
		return NULL;

	/* The two FIFOs are created. */

	if((fileDes_r = mkopFifo(CLI_FIFO_R_S, ret->clientName_r, 0666, O_RDWR))
			== ERR_FIFO)
	{
		free(ret);
		return NULL;
	}
	ret->clientFifo_r = fileDes_r;

	if((fileDes_w = mkopFifo(CLI_FIFO_W_S, ret->clientName_w, 0666, O_RDWR))
			== ERR_FIFO)
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

	id = getpid();
	mesg.id = id;
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

	msg.size = 1;
	msg.message = &c;
	rcvMsg(ret, &msg, 0);

	return ret;
}



comuADT getClient(servADT serv, pid_t id)
{
	infoClient matchingClient;
	infoClient client;
	void *arrayMatching = vArray_search(serv->clients, (int (*)(void *, void *))infoClient_comparePid, &client);

	client.id = id;
	client.comm = NULL;

	if(arrayMatching == NULL)
		return NULL;

	matchingClient = *(infoClient *)arrayMatching;

	return matchingClient.comm;
}



int sendMsg(comuADT comm, message *msg, int flags)
{
	int ret;

	if(flags == IPC_NOWAIT)
	{
		int auxFlags = fcntl(comm->clientFifo_w, F_GETFL, 0);

		if(fcntl(comm->clientFifo_w, F_SETFL, O_NONBLOCK) == -1)
		{
			fprintf(stderr, "Error on unblocking file descriptor.\n");
			return -1;
		}

		ret = write(comm->clientFifo_w, msg->message, msg->size);

		if(fcntl(comm->clientFifo_w, F_SETFL, auxFlags) == -1)
		{
			fprintf(stderr, "Error on blocking file descriptor.\n");
			return -1;
		}
	}
	else
		ret = write(comm->clientFifo_w, msg->message, msg->size);

	return ret;
}



int rcvMsg(comuADT comm, message *msg, int flags)
{
	int ret;

	if(comm == NULL || msg == NULL)
	{
		fprintf(stderr, "NULL parameters.\n");
		return -1;
	}


	if(flags == IPC_NOWAIT)
	{
		int auxFlags = fcntl(comm->clientFifo_r, F_GETFL, 0);

		if(fcntl(comm->clientFifo_r, F_SETFL, O_NONBLOCK) == -1)
		{
			fprintf(stderr, "Error on unblocking file descriptor.\n");
			return -1;
		}

		ret = read(comm->clientFifo_r, msg->message, msg->size);

		if(fcntl(comm->clientFifo_r, F_SETFL, auxFlags) == -1)
		{
			fprintf(stderr, "Error on blocking file descriptor.\n");
			return -1;
		}
	}
	else
		ret = read(comm->clientFifo_r, msg->message, msg->size);

	return ret;
}


int disconnectFromServer(comuADT comm, servADT server)
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



int endServer(servADT server)
{
	int i;

	if(server == NULL)
		return -1;

	serverConnected = FALSE;

	if(close(server->serverFifo) == -1)
	{
		fprintf(stderr, "Server FIFO couldn't be closed.\n");
		return -1;
	}

	if(unlink(server->serverName) == -1)
	{
		fprintf(stderr, "Server FIFO couldn't be removed.\n");
		return -1;
	}

	for(i = 0; i < server->clientsUsed; i++)
	{
		infoClient * icp = (infoClient *)vArray_getAt(server->clients, i);
		comuADT currentComm = icp->comm;

		close(currentComm->clientFifo_r);
		close(currentComm->clientFifo_w);

		if(unlink(currentComm->clientName_r) == -1 ||
				unlink(currentComm->clientName_w) == -1)
		{
			fprintf(stderr, "Client FIFO couldn't be removed.\n");
			return -1;
		}

		free(currentComm);
		free(icp);

	}

	vArray_destroy(server->clients);

	free(server);

	return 0;
}


int infoClient_comparePid(infoClient * ic1, infoClient * ic2)
{
	if(ic1 == ic2)
		return 0;
	if(ic1 == NULL || ic2 == NULL)
		return 1;
	
	if( ic1->id == ic2->id)
        return 0;
    else
        return 1;
}

void itoa(int n, char *string)
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        string[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        string[i++] = '-';
    string[i] = '\0';
    reverse(string);
}

void reverse(char *string)
{
    int i, j;
    char c;

    for (i = 0, j = strlen(string)-1; i<j; i++, j--) {
        c = string[i];
        string[i] = string[j];
        string[j] = c;
    }
}
