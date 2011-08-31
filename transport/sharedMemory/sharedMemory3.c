/***
***
***		sharedMemory.c
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/


/***		System includes		***/
#include <sys/sem.h>
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

/***		Project Includes		***/
#include "../../include/api.h"
#include "../../include/semaphore.h"
#include "../../include/shm.h"

/***		Functions		***/
void reverse(char *string);
void itoa(int n, char *string);


/*
 * Name: zeroOut
 * Receives: void * mem, int bytes
 * Returns: void
 * Description: This function zeroes out a certain amount of memory,
 * starting from mem until mem + bytes - 1.
 */
static void zeroOut(void * mem, int bytes) {
	int i;
	char * m = (char *) mem;
	if (mem == NULL)
		return;
	for (i = 0; i < bytes; i++)
		m[i] = 0;
}

/*
 * Functions
 */

/* Starts a Server */
servADT startServer() {
	int shmidClients = -1;
	int shmidMemory = -1;
	int semid = -1;
	int i = 0;
	void * clients;
	void * memory;
	int permits = 0666;

	servADT serv = malloc(sizeof(struct serverCDT));
	if (serv == NULL)
	{
		fprintf(stderr, "Server: Malloc returned null.");
		return NULL;
	}
	/* Starting the semaphores */
	semid = initSem();
	if (semid == -1) {
		free(serv);
		fprintf(stderr, "Server: Semaphore initialization failed\n");
		return NULL;
	}
	serv->semid = semid;

	/* Obtaining the memory for the client table */
	while ((shmidClients = shmget(KEY_1 + i, SIZE_CLIST, PERMFLAGS)) == -1)
		i++;

	i = 0;
	/* Obtaining the communication memory */
	while ((shmidMemory = shmget(KEY_2 + i, SIZE, PERMFLAGS)) == -1)
		i++;

	serv->shmidClients = shmidClients;
	serv->shmidMemory = shmidMemory;

	/* Attaching memory */
	clients = shmat(serv->shmidClients, NULL, permits | IPC_CREAT | IPC_EXCL);

	if (clients == (void*) -1) {
		if (errno == EEXIST) {
			clients = shmat(serv->shmidClients, NULL, 0);
			if (clients == (void*) -1) {
				fprintf(
						stderr,
						"Server: Error attaching shared memories with server process\n");
				free(serv);
				return NULL;
			}
			printf("Server: Attach to client table successful\n");
		} else {
			fprintf(
					stderr,
					"Server: Error attaching shared memories with server process\n");
			free(serv);
			return NULL;
		}
	}

	memory = shmat(serv->shmidMemory, NULL, permits | IPC_CREAT | IPC_EXCL);

	if (memory == (void*) -1) {
		if (errno == EEXIST) {
			memory = shmat(serv->shmidMemory, NULL, 0);
			if (memory == (void*) -1) {
				fprintf(stderr, "Server: Error associating shared memories "
						"with server process\n");
				free(serv);
				return NULL;
			}
			printf("Server: Attach to memory successful\n");
		} else {
			fprintf(stderr, "Server: Error associating shared memories "
					"with server process\n");
			free(serv);
			return NULL;
		}
	}

	/* Cleaning dirty memory -- I don't care what was written there */
	zeroOut(clients, SIZE_CLIST);
	zeroOut(memory, SIZE);

	serv->maxClients = MAX_CLIENTS;
	/*    REMOVE
	 comuADT * clients2 = (comuADT *)clients;
	 int w;
	 for (; w<serv->maxClients;w++){
	 (clients2[w])->id = 0 ;
	 (clients2[w])->used = 0;
	 }
	 REMOVE   */

	serv->clients = (void *) clients;
	serv->memory = memory;
	serv->msgSize = sizeof(sharedMemoryMessage);
	serv->isOnline = TRUE;


	return serv;
}

/* Connects a client to a given server */
comuADT connectToServer(servADT serv) {
	struct IPCCDT * clients;
	int permits = 0666;
	int i;
	comuADT comm = NULL;
	int flag = TRUE;


	if (serv == NULL)
	{
		fprintf(stderr, "Client: Cannot connect to NULL server\n");
		return NULL;
	}

	/* Requesting exclusivity */
	if (p(serv->semid, SEM_TABLE, TRUE) == -1)
		return NULL;

	/* Attaching memory */
	clients = shmat(serv->shmidClients, NULL, permits | IPC_CREAT | IPC_EXCL);
	if (clients == (void*) -1) {
		fprintf(
				stderr,
				"Client: Error associating shared memory with client process\n");
		if (errno == EEXIST) {
			clients = shmat(serv->shmidClients, NULL, 0);
			if (clients == (void*) -1) {
				fprintf(
						stderr,
						"Error associating shared memories with client process\n");
				return NULL;
			}
			printf("Client: The shared client table existed "
					"and attaching succeded\n");
		} else {
			fprintf(stderr, "Error associating shared memories "
					"with client process\n");
			return NULL;
		}
	}

	/* Selecting the first free client on the server client table */
	for (i = 0; i < serv->maxClients; i++) {
		if (clients[i].id == 0) {
			clients[i].id = getpid();
			clients[i].semid = serv->semid;
			clients[i].shmidMemory = serv->shmidMemory;
			clients[i].offset = i * 2 * sizeof(sharedMemoryMessage);
			clients[i].used = TRUE;
			break;
		}
	}
	if (i == serv->maxClients)
		return NULL;

	/* Abandon exclusivity */
	v(serv->semid, SEM_TABLE);


	while (flag) {
		/* Requesting exclusivity */
		if (p(serv->semid, SEM_TABLE, TRUE) == -1)
			return NULL;
		/* checking if the thread has initialized the client */
		if (clients[i].used == TRUE)
		{
			flag = FALSE;
		} else {

			/* Abandon exclusivity */
			v(serv->semid, SEM_TABLE);
			usleep(20000);

		}

	}

	/* initializing the client info */
	comm = malloc(sizeof(struct IPCCDT));
	if (comm == NULL)
	{
		fprintf(stderr, "Malloc returned NULL in connectToServer\n");
		return NULL;
	}

	comm->semid = (clients[i]).semid;
	comm->isServer = FALSE;
	comm->shmidMemory = (clients[i]).shmidMemory;
	comm->offset = (clients[i]).offset;
	/* linking the shared memory where the client s supposed to write and read */
	comm->memory = shmat(serv->shmidMemory, NULL,
			permits | IPC_CREAT | IPC_EXCL);

	if (comm->memory == (void*) -1) {
		if (errno == EEXIST) {
			comm->memory = shmat(serv->shmidMemory, NULL, 0);
			if (comm->memory == (void*) -1) {
				fprintf(stderr, "Error associating shared memories "
						"with client process\n");
				free(comm);
				return NULL;
			}
			printf(
					"Client: The shared memory existed and attaching succeded\n");
		} else {
			fprintf(stderr, "Error associating shared memories "
					"with client process\n");
			free(comm);
			return NULL;
		}
	}

	/* Detaching client table */
	/*shmdt(clients); */
	/* Abandon exclusivity */
	v(serv->semid, SEM_TABLE);

	/*printf("inside connectToServer\n");*/

	return comm;
}

/* Gets a client from the server side */
comuADT getClient(servADT server, pid_t id) {
	struct IPCCDT * clients = (struct IPCCDT *) server->clients;
	int i;
	int permits = 0666;
	comuADT comm = NULL;

	/* Requesting exclusivity */
	if (p(server->semid, SEM_TABLE, TRUE) == -1) {
		fprintf(stderr, "p op fail\n");
		return NULL;
	}
	for (i = 0; i < server->maxClients; i++) {
		if (clients[i].used == FALSE) {
			/* Abandon exclusivity */
			v(server->semid, SEM_TABLE);
			return NULL;
		} else if (clients[i].id == id && clients[i].used == TRUE)
		{
			comm = malloc(sizeof(struct IPCCDT));
			if (comm == NULL)
			{
				fprintf(stderr, "Malloc returned NULL in getClient\n");
			}

			comm->isServer = TRUE;
			comm->semid = (clients[i]).semid;
			comm->shmidMemory = (clients[i]).shmidMemory;
			comm->offset = (clients[i]).offset;
			comm->memory = shmat(comm->shmidMemory, NULL,
					permits | IPC_CREAT | IPC_EXCL);
			if (comm->memory == (void*) -1) {
				if (errno == EEXIST) {
					comm->memory = shmat(comm->shmidMemory, NULL, 0);
					if (comm->memory == (void*) -1) {
						fprintf(stderr, "Error associating shared memories "
								"with server process Errno: %d\n", errno);
						free(comm);
						return NULL;
					}
				} else {
					fprintf(stderr, "Error associating shared memories "
							"with server process lala Errno: %d\n", errno);
					free(comm);
					return NULL;
				}
			}
			break;
		}
	}
	/* Abandon exclusivity */
	v(server->semid, SEM_TABLE);

	return comm;
}

/* Sends a message through comm */
int sendMsg(comuADT comm, message * msg, int flags) {
	int amtSent = 0;
	char * origin = (char *) (msg->message);
	void * destination;
	sharedMemoryMessage * sending;

	if (msg == NULL || comm == NULL)
	{
		fprintf(stderr, "Invalid communication parameters\n");
		return -1;
	}

	sending = malloc(sizeof(sharedMemoryMessage));

	if (sending == NULL) {
		fprintf(stderr, "Malloc returned null in sendMsg\n");
		return -1;
	}

	/* Requesting exclusivity */
	if (p(comm->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
		free(sending);
		return -1;
	}
	/* variable used to write at the client's reserved position */
	/*destination = comm->memory + comm->offset
			+ ((comm->isServer) ? 0 : sizeof(sharedMemoryMessage));*/
	destination = comm->memory + comm->offset;

	/*blocks the procces if the massage table is full*/
	while (((sharedMemoryMessage*) destination)->isNotRead == TRUE
			&& ((sharedMemoryMessage*) destination)->amount != 0) {
		/* Abandon exclusivity */
		v(comm->semid, SEM_MEMORY);

		usleep(20000);

		/* Requesting exclusivity */
		p(comm->semid, SEM_MEMORY, TRUE);

	}

	sending->isNotRead = TRUE;
	amtSent = (msg->size > MESG_SIZE) ? MESG_SIZE : msg->size;
	sending->amount = amtSent;
	/*printf("sent amount : %d\n", amtSent);*/
	/* Packaging */
	memcpy(sending->message, origin, amtSent);
	/*amtSent = 10;
	strncpy(sending->message, origin, amtSent);*/
	/* Writing package */
	memcpy(destination, sending, sizeof(sharedMemoryMessage));

	free(sending);

	/* Abandon exclusivity */
	v(comm->semid, SEM_MEMORY);

	return amtSent;
}

/* Receives a message through comm */
int rcvMsg(comuADT comm, message * msg, int flags) {
	int amtRcv = 0;
	void * origin;
	sharedMemoryMessage * receiving;
	int done;


	if (msg == NULL || comm == NULL)
	{
		fprintf(stderr, "Invalid communication parameters\n");
		return -1;
	}

	receiving = malloc(sizeof(sharedMemoryMessage));

	if (receiving == NULL) {
		/*TODO: Free any remaining struct.. no point in going on*/
		fprintf(stderr, "Malloc returned null in rcvMsg\n");
		return -1;
	}
	/* Requesting exclusivity */
	if (p(comm->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
		free(receiving);
		return -2;
	}
	/*origin = comm->memory + comm->offset
				+ ((comm->isServer) ? sizeof(sharedMemoryMessage) : 0);*/

	origin = comm->memory + comm->offset;


	memcpy(receiving, origin, sizeof(sharedMemoryMessage));
	if (flags == IPC_NOWAIT) {
		if (receiving->isNotRead == FALSE)
		{
			free(receiving);
			/* Abandon exclusivity */
			v(comm->semid, SEM_MEMORY);
			return 0;
		}
	} else {
		/* Abandon exclusivity */
		v(comm->semid, SEM_MEMORY);
		done = FALSE;
		while (!done) {
			/* Requesting exclusivity */
			if (p(comm->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
				free(receiving);
				return 0;
			}
			memcpy(receiving, origin, sizeof(sharedMemoryMessage));

			if (receiving->isNotRead == TRUE)
			{
				done = TRUE;
			} else {
				/* Abandon exclusivity */
				v(comm->semid, SEM_MEMORY);
				usleep(20000);
			}
		}
	}

	/*printf("received amount : %d\n", receiving->amount);
	printf("received string: %s\n", (char *)receiving->message );*/
	msg->message = malloc ( sizeof(char) * receiving->amount );
	memcpy(msg->message, receiving->message, receiving->amount);
	/*printf("received string: %s\n", (char *)msg->message );*/
	msg->size = receiving->amount;
	amtRcv = receiving->amount;

	free(receiving);
	((sharedMemoryMessage*) origin)->isNotRead = FALSE;
	((sharedMemoryMessage*) origin)->amount = 0;

	/* Abandon exclusivity */
	v(comm->semid, SEM_MEMORY);
	return amtRcv;
}

/* Disconnects a client from a server */
int disconnectFromServer(comuADT comm, servADT server) {
	shmdt(comm->memory);
	free(comm);
	return 0;
}

/* Ends a server */
int endServer(servADT server) {
	server->isOnline = FALSE;
	shmdt(server->memory);
	shmdt(server->clients);
	shmctl(server->shmidClients, IPC_RMID, NULL);
	shmctl(server->shmidMemory, IPC_RMID, NULL);
	destroySem(server->semid);
	free(server);
	return 0;
}

/*
 * Receives a string and stocks in it the reversed representation of it.
 */
void reverse(char *string) {
	int i, j;
	char c;

	for (i = 0, j = strlen(string) - 1; i < j; i++, j--) {
		c = string[i];
		string[i] = string[j];
		string[j] = c;
	}
}

/*
 * Stores in string the string-representation of n.
 */
void itoa(int n, char *string) {
	int i, sign;

	if ((sign = n) < 0) /* record sign */
		n = -n; /* make n positive */
	i = 0;
	do { /* generate digits in reverse order */
		string[i++] = n % 10 + '0'; /* get next digit */
	} while ((n /= 10) > 0); /* delete it */
	if (sign < 0)
		string[i++] = '-';
	string[i] = '\0';
	reverse(string);
}

