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
#include <unistd.h>

/***		Project Includes		***/
#include "../../include/api.h"
#include "../../include/semaphore.h"
#include "../../include/sharedMemory.h"

/***		Functions		***/


static void cleanUP(void * mem, int bytes) {
	int i;
	char * m = (char *) mem;
	if (mem == NULL)
		return;
	for (i = 0; i < bytes; i++)
		m[i] = 0;
}


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
	semid = initSem(1);
	if (semid == -1) {
		free(serv);
		fprintf(stderr, "Server: Semaphore initialization failed.");
		return NULL;
	}
	serv->semid = semid;

	/* Obtaining the memory for the client table */
	while ((shmidClients = shmget(KEY_1 + i, SIZE_CLIST, FLAGS)) == -1)
		i++;

	i = 0;
	/* Obtaining the communication memory */
	while ((shmidMemory = shmget(KEY_2 + i, SIZE, FLAGS)) == -1)
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

	/* Cleaning dirty memory */
	cleanUP(clients, SIZE_CLIST);
	cleanUP(memory, SIZE);

	serv->maxClients = MAX_CLIENTS;
	serv->clients = (void *) clients;
	serv->memory = memory;

	return serv;
}

comuADT connectToServer(servADT serv) {
	struct clientCDT * clients;
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
	if (up(serv->semid, SEM_CLI_TABLE, TRUE) == -1)
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
	down(serv->semid, SEM_CLI_TABLE);


	while (flag) {
		/* Requesting exclusivity */
		if (up(serv->semid, SEM_CLI_TABLE, TRUE) == -1)
			return NULL;
		if (clients[i].used == TRUE)
		{
			flag = FALSE;
		} else {

			/* Abandon exclusivity */
			down(serv->semid, SEM_CLI_TABLE);
			/*TODO usleep(20000);*/

		}

	}

	/* initializing the client info */
	comm = malloc(sizeof(struct clientCDT));
	if (comm == NULL)
	{
		fprintf(stderr, "Malloc returned NULL in connectToServer\n");
		return NULL;
	}

	comm->semid = (clients[i]).semid;
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
	down(serv->semid, SEM_CLI_TABLE);

	return comm;
}

comuADT getClient(servADT server, pid_t id) {
	struct clientCDT * clients = (struct clientCDT *) server->clients;
	int i;
	int permits = 0666;
	comuADT comm = NULL;

	/* Requesting exclusivity */
	if (up(server->semid, SEM_CLI_TABLE, TRUE) == -1) {
		fprintf(stderr, "p op fail\n");
		return NULL;
	}
	for (i = 0; i < server->maxClients; i++) {
		if (clients[i].used == FALSE) {
			/* Abandon exclusivity */
			down(server->semid, SEM_CLI_TABLE);
			return NULL;
		} else if (clients[i].id == id && clients[i].used == TRUE)
		{
			comm = malloc(sizeof(struct clientCDT));
			if (comm == NULL)
			{
				fprintf(stderr, "Malloc returned NULL in getClient\n");
			}

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
	down(server->semid, SEM_CLI_TABLE);

	return comm;
}

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
	if (up(comm->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
		free(sending);
		return -1;
	}
	/* variable used to write at the client's reserved position */
	destination = comm->memory + comm->offset;

	/*blocks the process if the massage table is full*/
	while (((sharedMemoryMessage*) destination)->isWritten == TRUE
			&& ((sharedMemoryMessage*) destination)->amount != 0) {
		/* Abandon exclusivity */
		down(comm->semid, SEM_MEMORY);

		/*usleep(20000);TODO*/

		/* Requesting exclusivity */
		up(comm->semid, SEM_MEMORY, TRUE);

	}

	sending->isWritten = TRUE;
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
	down(comm->semid, SEM_MEMORY);

	return amtSent;
}

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
	if (up(comm->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
		free(receiving);
		return -2;
	}
	/*origin = comm->memory + comm->offset
				+ ((comm->isServer) ? sizeof(sharedMemoryMessage) : 0);*/

	origin = comm->memory + comm->offset;


	memcpy(receiving, origin, sizeof(sharedMemoryMessage));
	if (flags == IPC_NOWAIT) {
		if (receiving->isWritten == FALSE)
		{
			free(receiving);
			/* Abandon exclusivity */
			down(comm->semid, SEM_MEMORY);
			return 0;
		}
	} else {
		/* Abandon exclusivity */
		down(comm->semid, SEM_MEMORY);
		done = FALSE;
		while (!done) {
			/* Requesting exclusivity */
			if (up(comm->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
				free(receiving);
				return 0;
			}
			memcpy(receiving, origin, sizeof(sharedMemoryMessage));

			if (receiving->isWritten == TRUE)
			{
				done = TRUE;
			} else {
				/* Abandon exclusivity */
				down(comm->semid, SEM_MEMORY);
				/*TODO usleep(20000);*/
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
	((sharedMemoryMessage*) origin)->isWritten = FALSE;
	((sharedMemoryMessage*) origin)->amount = 0;

	/* Abandon exclusivity */
	down(comm->semid, SEM_MEMORY);
	return amtRcv;
}

int disconnectFromServer(comuADT comm, servADT server) {
	shmdt(comm->memory);
	free(comm);
	return 0;
}

int endServer(servADT server) {
	shmdt(server->memory);
	shmdt(server->clients);
	shmctl(server->shmidClients, IPC_RMID, NULL);
	shmctl(server->shmidMemory, IPC_RMID, NULL);
	destroySem(server->semid);
	free(server);
	return 0;
}

void reverse(char *string) {
	int i, j;
	char c;

	for (i = 0, j = strlen(string) - 1; i < j; i++, j--) {
		c = string[i];
		string[i] = string[j];
		string[j] = c;
	}
}

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

