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

serverADT startServer() {
	int shmidClients = -1;
	int shmidMessages = -1;
	int semid = -1;
	int i = 0;
	void * clients;
	void * memory;
	int permits = 0666;

	serverADT serv = malloc(sizeof(struct serverCDT));
	if (serv == NULL)
	{
		fprintf(stderr,
				"startServer(): not enough space to initialize serverADT.\n");
		return NULL;
	}
	/* Starting the semaphores */
	semid = initSem(1);
	if (semid == -1) {
		free(serv);
		fprintf(stderr, "startServer(): Semaphore initialization failed.\n");
		return NULL;
	}
	serv->semid = semid;

	/* Obtaining the memory for the client vector memory*/
	while ((shmidClients = shmget(KEY_1 + i, SIZE_CLI_VEC, FLAGS)) == -1)
		i++;

	/* Obtaining the clientunication memory */
	while ((shmidMessages = shmget(KEY_2 + i, SIZE, FLAGS)) == -1)
			i++;


	serv->shmidClients = shmidClients;
	serv->shmidMessages = shmidMessages;

	/* Attaching client vector memory */
	clients = shmat(serv->shmidClients, NULL, permits | IPC_CREAT | IPC_EXCL);

	if (clients == (void*) -1) {
		fprintf(stderr,
				"startServer(): error attaching memory to clients pointer.\n");
		free(serv);
		return NULL;
	}

	/* Attaching message memory */
	memory = shmat(serv->shmidMessages, NULL, permits | IPC_CREAT | IPC_EXCL);

	if (memory == (void*) -1) {
		fprintf(stderr,
				"startServer(): error attaching memory to memory pointer.\n");
		free(serv);
		return NULL;
	}

	/* Cleaning dirty memory */
	cleanUP(clients, SIZE_CLI_VEC);
	cleanUP(memory, SIZE);

	serv->maxClients = MAX_CLIENTS;
	serv->clients = (void *) clients;
	serv->memory = memory;

	return serv;
}

clientADT connectToServer(serverADT serv) {
	struct clientCDT * clients;
	int permits = 0666;
	int i;
	clientADT client = NULL;
	int flag = TRUE;

	if (serv == NULL)
	{
		fprintf(stderr, "connectToServer():  NULL server.\n");
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
				"connectToServer(): error attaching memory to clients pointer.\n");
		return NULL;
	}

	/* Selecting the first free client on the server client vector */
	for (i = 0; i < serv->maxClients; i++) {
		if (clients[i].id == 0) {
			clients[i].id = getpid();
			clients[i].semid = serv->semid;
			clients[i].shmidMessages = serv->shmidMessages;
			clients[i].offset = i * 2 *  sizeof(shmMessage);
			clients[i].used = TRUE;
			break;
		}
	}
	if (i == serv->maxClients){
		fprintf(
				stderr,
				"connectToServer():the client vector is full.\n");
		return NULL;
	}

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
		}

	}

	/* initializing the client info */
	client = malloc(sizeof(struct clientCDT));
	if (client == NULL)
	{
		fprintf(stderr, "Malloc returned NULL in connectToServer.\n");
		return NULL;
	}

	client->semid = (clients[i]).semid;
	client->shmidMessages = (clients[i]).shmidMessages;
	client->offset = (clients[i]).offset;
	/* linking the shared memory where the client s supposed to write and read */
	client->memory = shmat(serv->shmidMessages, NULL,
			permits | IPC_CREAT | IPC_EXCL);

	if (client->memory == (void*) -1) {
		fprintf(stderr,
				"connectToServer(): error attaching memory to memory pointer.\n");
		free(client);
		return NULL;
	}

	/* Detaching client table */
	shmdt(clients);
	/* Abandon exclusivity */
	down(serv->semid, SEM_CLI_TABLE);

	return client;
}

clientADT getClient(serverADT server, pid_t id) {
	struct clientCDT * clients = (struct clientCDT *) server->clients;
	int i;
	int permits = 0666;
	clientADT client = NULL;

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
			client = malloc(sizeof(struct clientCDT));
			if (client == NULL)
			{
				fprintf(stderr, "Malloc returned NULL in getClient.\n");
			}

			client->semid = (clients[i]).semid;
			client->shmidMessages = (clients[i]).shmidMessages;
			client->offset = (clients[i]).offset;
			client->memory = shmat(client->shmidMessages, NULL,
					permits | IPC_CREAT | IPC_EXCL);
			if (client->memory == (void*) -1) {
				if (errno == EEXIST) {
					client->memory = shmat(client->shmidMessages, NULL, 0);
					if (client->memory == (void*) -1) {
						fprintf(stderr, "Error associating shared memories "
								"with server process Errno: %d\n", errno);
						free(client);
						return NULL;
					}
				} else {
					fprintf(stderr, "Error associating shared memories "
							"with server process lala Errno: %d\n", errno);
					free(client);
					return NULL;
				}
			}
			break;
		}
	}
	/* Abandon exclusivity */
	down(server->semid, SEM_CLI_TABLE);

	return client;
}

int sendMsg(clientADT client, message * msg, int flags) {
	int amtSent = 0;
	char * origin = (char *) (msg->message);
	void * destination;
	shmMessage * sending;

	if (msg == NULL || client == NULL)
	{
		fprintf(stderr, "Invalid clientunication parameters\n");
		return -1;
	}

	sending = malloc(sizeof(shmMessage));

	if (sending == NULL) {
		fprintf(stderr, "Malloc returned null in sendMsg\n");
		return -1;
	}

	/* Requesting exclusivity */
	if (up(client->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
		free(sending);
		return -1;
	}
	/* variable used to write at the client's reserved position */
	destination = client->memory + client->offset;

	/*blocks the process if the massage table is full*/
	while (((shmMessage*) destination)->isWritten == TRUE
			&& ((shmMessage*) destination)->quantity != 0) {
		/* Abandon exclusivity */
		down(client->semid, SEM_MEMORY);


		/* Requesting exclusivity */
		up(client->semid, SEM_MEMORY, TRUE);

	}

	sending->isWritten = TRUE;
	amtSent = (msg->size > MESG_SIZE) ? MESG_SIZE : msg->size;
	sending->quantity = amtSent;
	/* Packaging */
	memcpy(sending->message, origin, amtSent);
	/* Writing package */
	memcpy(destination, sending, sizeof(shmMessage));

	free(sending);

	/* Abandon exclusivity */
	down(client->semid, SEM_MEMORY);

	return amtSent;
}

int rcvMsg(clientADT client, message * msg, int flags) {
	int amtRcv = 0;
	void * origin;
	shmMessage * receiving;
	int done;

	if (msg == NULL || client == NULL)
	{
		fprintf(stderr, "Invalid clientunication parameters\n");
		return -1;
	}

	receiving = malloc(sizeof(shmMessage));

	if (receiving == NULL) {
		/*TODO: Free any remaining struct.. no point in going on*/
		fprintf(stderr, "Malloc returned null in rcvMsg\n");
		return -1;
	}
	/* Requesting exclusivity */
	if (up(client->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
		free(receiving);
		return -2;
	}

	origin = client->memory + client->offset;

	memcpy(receiving, origin, sizeof(shmMessage));
	if (flags == IPC_NOWAIT) {
		if (receiving->isWritten == FALSE)
		{
			free(receiving);
			/* Abandon exclusivity */
			down(client->semid, SEM_MEMORY);
			return 0;
		}
	} else {
		/* Abandon exclusivity */
		down(client->semid, SEM_MEMORY);
		done = FALSE;
		while (!done) {
			/* Requesting exclusivity */
			if (up(client->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
				free(receiving);
				return 0;
			}
			memcpy(receiving, origin, sizeof(shmMessage));

			if (receiving->isWritten == TRUE)
			{
				done = TRUE;
			} else {
				/* Abandon exclusivity */
				down(client->semid, SEM_MEMORY);
				/*TODO usleep(20000);*/
			}
		}
	}

	/*msg->message = malloc(sizeof(char) * receiving->quantity);*/
	memcpy(msg->message, receiving->message, receiving->quantity);
	msg->size = receiving->quantity;
	amtRcv = receiving->quantity;

	free(receiving);
	((shmMessage*) origin)->isWritten = FALSE;
	((shmMessage*) origin)->quantity = 0;

	/* Abandon exclusivity */
	down(client->semid, SEM_MEMORY);
	return amtRcv;
}

int disconnectFromServer(clientADT client, serverADT server) {
	shmdt(client->memory);
	free(client);
	return 0;
}

int endServer(serverADT server) {
	shmdt(server->memory);
	shmdt(server->clients);
	shmctl(server->shmidClients, IPC_RMID, NULL);
	shmctl(server->shmidMessages, IPC_RMID, NULL);
	destroySem(server->semid);
	free(server);
	return 0;
}
