/***
***
***		fifo.c
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/
/*
 * Includes
 */

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
#include "../../include/api.h"


void itoa(int n, char *string);

void reverse(char *string);
/*
 * Symbolic constants definitions.
 */

/***		Module Defines		***/

#define	CLIENTS_SIZE	10
#define CHARS_ADD		10

#define SER_FIFO_S		"/tmp/serverFifo"
/*#define SER_FIFO_LEN	(strlen(SER_FIFO_S) + CHARS_ADD)*/
#define SER_FIFO_LEN	(27)

#define CLI_FIFO_R_S	"/tmp/clientFifo"
/*#define CLI_FIFO_R_LEN	(strlen(CLI_FIFO_R_S) + CHARS_ADD)*/
#define CLI_FIFO_R_LEN	(27)

#define CLI_FIFO_W_S	"/tmp/clientFifo_w"
/*#define CLI_FIFO_W_LEN	(strlen(CLI_FIFO_W_S) + CHARS_ADD)*/
#define CLI_FIFO_W_LEN	(27)

#define ERROR_FIFO		-1

void reverse(char *string);
void itoa(int n, char *string);

/*
 * Global variables.
 */

int serverConnected;


/*
 * Structs.
 */

struct IPCCDT
{
	int clientFifo_s;

	int clientFifo;
	char clientName_r[CLI_FIFO_R_LEN];
};

struct serverCDT
{
	int serverFifo;
	char serverName[SER_FIFO_LEN];

	int clientsUsed;
};

/*
 * Main functions
 */


int makeFifo( int pid, char * prefix, char * finalName, mode_t creationMode, mode_t operationMode ){
	
	int length;
	int fd;
	length = strlen(prefix);
	strcpy(finalName, prefix);
	itoa(pid, finalName + length);
	
	if(mkfifo(finalName, creationMode) == -1){
		perror("Fifo couldn't be created\n");
		return ERROR_FIFO;	
	}

	if((fd = open(finalName, operationMode)) == -1)
	{
		fprintf(stderr, "FIFO couldn't be opened.\n");
		return ERROR_FIFO;
	}

	return fd;
}

servADT startServer(void)
{
	int fileDescriptor;

	servADT server = (servADT)malloc(sizeof(struct serverCDT));
	if( server == NULL){
		perror("Fail creating server\n");
		return NULL;
	}

	if( ( fileDescriptor = makeFifo(getpid(), SER_FIFO_S, server->serverName, 0666,
			O_RDWR | O_NONBLOCK | O_APPEND ) ) == ERROR_FIFO ){
		perror("Fail creating FIFO\n");
		return NULL;	
	}

	server->serverFifo = fileDescriptor;

	serverConnected = TRUE;
	
	return server;
}


comuADT connectToServer(servADT serv)
{
	int read_fileDescriptor;
	int server_fileDescriptor = serv->serverFifo;
	int pid = getpid();

	comuADT comm = (comuADT)malloc(sizeof(struct IPCCDT));
	if( comm == NULL){
		perror("Fail creating comunicator\n");
		return NULL;
	}

	if( ( read_fileDescriptor = makeFifo(pid, CLI_FIFO_R_S, comm->clientName_r, 0666,
			O_RDWR | O_APPEND) ) == ERROR_FIFO ){
		perror("Fail creating reading FIFO\n");
		return NULL;	
	}

	/* Inicializo el resto de las variables que faltaban */

	comm->clientFifo = read_fileDescriptor;
	comm->clientFifo_s =  server_fileDescriptor;

	return comm;
}


comuADT getClient(servADT server, pid_t id)
{
	
	int r_fd;
	char newReadString[27];
	char * readString = CLI_FIFO_R_S;
	int rlen = strlen(readString);
	strcpy(newReadString, readString);
	itoa(id, newReadString + rlen);

	comuADT comm = (comuADT)malloc(sizeof(struct IPCCDT));
	if( comm == NULL){
		perror("Fail creating comunicator\n");
		return NULL;
	}

	if((r_fd = open(newReadString, O_RDWR )) == -1)
	{
		fprintf(stderr, "FIFO couldn't be opened.\n");
		return NULL;
	}
	
	comm->clientFifo = r_fd;
	comm->clientFifo_s = server->serverFifo;
	strcpy(comm->clientName_r, newReadString);	

	return comm;
}



int sendMsg(comuADT comm, message *msg, int flags)
{
	int ret;

	if(flags == IPC_NOWAIT)
	{
		int auxFlags = fcntl(comm->clientFifo, F_GETFL, 0);

		if(fcntl(comm->clientFifo, F_SETFL, O_NONBLOCK) == -1)
		{
			fprintf(stderr, "Error on unblocking file descriptor.\n");
			return -1;
		}

		ret = write(comm->clientFifo, msg->message, msg->size);

		if(fcntl(comm->clientFifo, F_SETFL, auxFlags) == -1)
		{
			fprintf(stderr, "Error on blocking file descriptor.\n");
			return -1;
		}
	}
	else
		ret = write(comm->clientFifo, msg->message, msg->size);

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
		int auxFlags = fcntl(comm->clientFifo, F_GETFL, 0);

		if(fcntl(comm->clientFifo, F_SETFL, O_NONBLOCK) == -1)
		{
			fprintf(stderr, "Error on unblocking file descriptor.\n");
			return -1;
		}

		ret = read(comm->clientFifo, msg->message, msg->size);

		if(fcntl(comm->clientFifo, F_SETFL, auxFlags) == -1)
		{
			fprintf(stderr, "Error on blocking file descriptor.\n");
			return -1;
		}

	}
	else
		ret = read(comm->clientFifo, msg->message, msg->size);

	return ret;
}


int disconnectFromServer(comuADT comm, servADT server)
{
	if(comm == NULL || server == NULL)
		return -1;

	if(close(comm->clientFifo) == -1)
	{
		fprintf(stderr, "Client couldn't be closed.\n");
		return -1;
	}

	free(comm);

	return 0;
}



int endServer(servADT server)
{
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

	free(server);

	return 0;
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

