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
#include "../../include/varray.h"


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
#define SER_FIFO_LEN	(25)

#define CLI_FIFO_R_S	"/tmp/clientFifo_r"
/*#define CLI_FIFO_R_LEN	(strlen(CLI_FIFO_R_S) + CHARS_ADD)*/
#define CLI_FIFO_R_LEN	(22)

#define CLI_FIFO_W_S	"/tmp/clientFifo_w"
/*#define CLI_FIFO_W_LEN	(strlen(CLI_FIFO_W_S) + CHARS_ADD)*/
#define CLI_FIFO_W_LEN	(22)

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

	int clientFifo_r;
	char clientName_r[CLI_FIFO_R_LEN];

/*	int clientFifo_w;
//	char clientName_w[CLI_FIFO_W_LEN];*/
};

struct serverCDT
{
	int serverFifo;
	char serverName[SER_FIFO_LEN];

	int clientsUsed;
};


typedef struct
{
	pid_t id;
	
	int clientFifo_r;
	char clientName_r[CLI_FIFO_R_LEN];

	int clientFifo_w;
	char clientName_w[CLI_FIFO_W_LEN];

} connectionMsg;

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
			O_RDWR | O_NONBLOCK ) ) == ERROR_FIFO ){
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
	/*int write_fileDescriptor;*/
	int server_fileDescriptor = serv->serverFifo;
	int pid = getpid();

	comuADT comm = (comuADT)malloc(sizeof(struct IPCCDT));
	if( comm == NULL){
		perror("Fail creating comunicator\n");
		return NULL;
	}

	/*Creating Fifo for Writing*/
	
	/*if( ( write_fileDescriptor = makeFifo(pid, CLI_FIFO_W_S, comm->clientName_w, 0666,
			O_RDWR ) ) == ERROR_FIFO ){
		perror("Fail creating writing FIFO\n");
		return NULL;	
	}*/
	
	/*Creating Fifo for reading*/
	
	if( ( read_fileDescriptor = makeFifo(pid, CLI_FIFO_R_S, comm->clientName_r, 0666,
			O_RDWR ) ) == ERROR_FIFO ){
		perror("Fail creating reading FIFO\n");
		return NULL;	
	}

	/* Inicializo el resto de las variables que faltaban */

	comm->clientFifo_r = read_fileDescriptor;
	/*comm->clientFifo_w = write_fileDescriptor;*/
	comm->clientFifo_s =  server_fileDescriptor;

	return comm;
}


comuADT getClient(servADT server, pid_t id)
{
	int w_fd;
	int r_fd;
	char newWriteString[27];
	char newReadString[27];
	char * writeString = CLI_FIFO_W_S;
	char * readString = CLI_FIFO_R_S;
	int wlen = strlen(writeString);
	int rlen = strlen(readString);
	strcpy(newWriteString, writeString);
	strcpy(newReadString, readString);
	itoa(id, newWriteString + wlen);
	itoa(id, newReadString + rlen);

	comuADT comm = (comuADT)malloc(sizeof(struct IPCCDT));
	if( comm == NULL){
		perror("Fail creating comunicator\n");
		return NULL;
	}

	
	
	/*if((w_fd = open(newWriteString, O_RDWR)) == -1)
	{
		fprintf(stderr, "FIFO couldn't be opened.\n");
		return NULL;
	}*/

	if((r_fd = open(newReadString, O_RDWR)) == -1)
	{
		fprintf(stderr, "FIFO couldn't be opened.\n");
		return NULL;
	}

	comm->clientFifo_r = r_fd;
	/*comm->clientFifo_w = w_fd;*/
	comm->clientFifo_s = server->serverFifo;
	/*strcpy(comm->clientName_w, newWriteString);*/
	strcpy(comm->clientName_r, newReadString);

	return comm;
}



int sendMsg(comuADT comm, message *msg, int flags)
{
	int ret;

	if(flags == IPC_NOWAIT)
	{
		int auxFlags = fcntl(comm->clientFifo_r, F_GETFL, 0);

		if(fcntl(comm->clientFifo_r, F_SETFL, O_NONBLOCK) == -1)
		{
			fprintf(stderr, "Error on unblocking file descriptor.\n");
			return -1;
		}

		ret = write(comm->clientFifo_r, msg->message, msg->size);

		if(fcntl(comm->clientFifo_r, F_SETFL, auxFlags) == -1)
		{
			fprintf(stderr, "Error on blocking file descriptor.\n");
			return -1;
		}
	}
	else
		ret = write(comm->clientFifo_r, msg->message, msg->size);

	return ret;	
	/*int ret;
	ret = write(comm->clientFifo_w, msg->message, msg->size);
	return ret;*/
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

	/*int ret;
	ret = read( comm->clientFifo_r,msg->message,msg->size);
	return ret;*/
}


int disconnectFromServer(comuADT comm, servADT server)
{
	return 0;
}



int endServer(servADT server)
{
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

pid_t pids[10];
int status;

int main() {
	message apimsg, anothermsg;
	comuADT client, rcvClient, sndClient;
	servADT server;
	int k = 1, i = 0, n;
	int sonsCount = 3;
	int turns = 2;

	server = startServer();
	client = connectToServer(server);

	/*initialization of all sons*/
	while (k < sonsCount) {
		switch (pids[k] = fork()) {
		case -1:
			perror("error in son conception");
			break;
		case 0:
			printf("I am son No %d\n", k);
			/*int rndtime;
			unsigned int iseed = (unsigned int)time(NULL);
			srand (iseed);*/
			client = connectToServer(server);

			while (1) {
				raise(SIGSTOP);
				apimsg.message = malloc(20);
				anothermsg.message = malloc(20);
				n = rcvMsg(client, &apimsg, 0);
				printf("Son %d: I've received %s !\n", k, (char *) apimsg.message);
				printf("%d chars\n", n);
				sndClient = getClient(server, getppid());
				sprintf(anothermsg.message, "send U this: %d", atoi(apimsg.message) );
				anothermsg.size = 20;
				printf("apimsg values: %s, %ld\n", (char*)anothermsg.message, anothermsg.size);
				printf("send from child: %d\n", sendMsg(client, &anothermsg, 0));
				printf("prueba %ld\n", anothermsg.size);
			}
			_exit(0);
			break;
		}
		k++;
	}
	sleep(1);

	/*delete*/
	int j = 1;
	while (j < 3) {
		printf("%d\n", pids[j]);
		j++;
	}
	/*delete*/

	sleep(2);
	i = 1;
	char buf[5];
	char string[20];
	strcpy(string, "you are son  ");
	printf("I am tha father and lord\n");
	client = connectToServer(server);
	while (turns) {
		/* does the broadcast */
		printf("\nbroadcasting\n");
		while (i < sonsCount) {
			itoa(i, buf);
			printf("%s\n", buf);
			apimsg.message = malloc(20);
			strcpy(apimsg.message, buf);
			apimsg.size = 20;
			sndClient = getClient(server, pids[i]);
			sendMsg(sndClient, &apimsg, 0);
			kill(pids[i], SIGCONT);
			i++;
			sleep(1);
		}
		i = 1;
		sleep(5);
		printf("\nreceiving messages\n");
		/* recieves the messages */
		while (i < sonsCount) {
			n = rcvMsg(client, &apimsg, /*IPC_NOWAIT*/ 0);
			printf("Father: I've received %s !\n", (char *) apimsg.message);
			printf("%d chars\n", n);
			sleep(1);
			i++;
		}
		i=1;
		turns--;
		sleep(1);

	}
	/*kills running processes before quitting*/
	int q=1;
	 while(q<sonsCount){
	 kill(pids[q], SIGTERM);
	 q++;
	 }
	printf("father running\n");
	return 0;
}

void fatal(char *s);

void fatal(char *s) {
	perror(s);
	exit(1);
}
