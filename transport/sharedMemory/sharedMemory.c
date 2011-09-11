/***
 ***
 ***		sharedMemory.c
 ***		Jose Ignacio Galindo
 ***		Federico Homovc
 ***		Nicolas Loreti
 ***		ITBA 2011
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
#include "../../include/transport.h"
#include "../../include/semaphore.h"

/*
 * Symbolic constants
 */
#define KEY_1 (key_t)0x1000
#define KEY_2 (key_t)0x10000
/* The maximum message size in bytes */
#define MESG_SIZE 4096
/* The maximum quantity of clients that the IPC can handle */
#define MAX_CLIENTS 20
#define SIZE_CLI_VEC ((size_t)(sizeof(clientADT) * (MAX_CLIENTS + 1)))
#define SIZE ((size_t)(sizeof(shmMessage)*MAX_CLIENTS*2))

#define SEM_CLI_TABLE 0
#define SEM_MEMORY 1

#define PERMS 0666 | IPC_CREAT | IPC_EXCL


/*
 * name: clientCDT
 * description: is the implementation of serverADT. It stores the necessary
 * information for a client to connect to the server and to other clients.
 * @id: unique identification of a client. It holds the pid of the process.
 * @used: flag to indicate whether this position in the client vector is empty
 *  or not(it is not used for communication).
 * @semid: the semaphore ID, used to be able to mutually exclude access to the
 * shared memory  message`s vector.
 * @shmidMessages: the shared memory ID to be able to access the message vector.
 * @offset: The offset (of the message vector) assigned for communication.
 * @memory: the pointer to the shared memory already attached.
 */
struct clientCDT {
	pid_t id;
	int used;
	int semid;
	int shmidMessages;
	int offset;
	void * memory;
};

/*
 * name: serverCDT
 * description: is the implementation of serverADT. It stores the necessary
 * information to connect to a server.
 * @semid: the semaphore ID, used to be able to mutually exclude access to the
 * shared memory vectors (clients and messages).
 * @shmidClients: The shared memory ID of the client's vector.
 * @shmidMessages: The shared memory ID of the message's vector.
 * @clients: The pointer to the shared memory client vector already attached.
 * @memory: The pointer to the shared memory message vector already attached.
 */
struct serverCDT {
	int semid;
	int shmidClients;
	int shmidMessages;
	void * clients;
	void * memory;
};

/*
 * name: shmMessage
 * description:  stands for a message stored in the message shared memory
 * @isWritten: boolean variable that indicates whether the message have
 * been written (is full) or not.
 * @quantity: specifies the length of the message.
 * @message: a char array with the message.
 */
typedef struct shmMessage {
	int isWritten;
	int quantity;
	char message[MESG_SIZE];
} shmMessage;

/***		Functions		***/

static void cleanUP(void * mem, int bytes) {
	int i;
	char * m = (char *) mem;
	if (mem == NULL){
		return;
	}
	for (i = 0; i < bytes; i++)
		m[i] = 0;
}

serverADT createServer() {
	int semid = -1;
	int shmidClients = -1;
	int shmidMessages = -1;
	void * clients;
	void * memory;
	int i = 0;

	serverADT serv = malloc(sizeof(struct serverCDT));
	if (serv == NULL)
	{
		fprintf(stderr,
				"createServer(): not enough space to initialize serverADT.\n");
		return NULL;
	}
	/* Starting the semaphores with initial value 1 */
	semid = initSem(1);
	if (semid == -1) {
		free(serv);
		fprintf(stderr, "createServer(): Semaphore initialization failed.\n");
		return NULL;
	}
	serv->semid = semid;

	/* Obtaining the memory for the client vector memory*/
	while ((shmidClients = shmget(KEY_1 + i, SIZE_CLI_VEC, FLAGS)) == -1)
		i++;

	/* Obtaining the client communication memory */
	while ((shmidMessages = shmget(KEY_2 + i, SIZE, FLAGS)) == -1)
		i++;

	serv->shmidClients = shmidClients;
	serv->shmidMessages = shmidMessages;

	/* Attaching client vector memory */
	clients = shmat(serv->shmidClients, NULL, PERMS);

	if (clients == (void*) -1) {
		fprintf(stderr,
				"createServer(): error attaching memory to clients pointer.\n");
		free(serv);
		return NULL;
	}

	/* Attaching message memory */
	memory = shmat(serv->shmidMessages, NULL, PERMS);

	if (memory == (void*) -1) {
		fprintf(stderr,
				"createServer(): error attaching memory to memory pointer.\n");
		free(serv);
		return NULL;
	}

	/* Cleaning dirty memory */
	cleanUP(clients, SIZE_CLI_VEC);
	cleanUP(memory, SIZE);

	serv->clients = (void *) clients;
	serv->memory = memory;

	return serv;
}

clientADT connectToServer(serverADT serv) {
	struct clientCDT * clients;
	int i;
	clientADT client = NULL;

	if (serv == NULL)
	{
		fprintf(stderr, "connectToServer():  NULL server.\n");
		return NULL;
	}

	if (up(serv->semid, SEM_CLI_TABLE, TRUE) == -1)
		return NULL;

	/* Attaching memory */
	clients = shmat(serv->shmidClients, NULL, PERMS);
	if (clients == (void*) -1) {
		fprintf(
				stderr,
				"connectToServer(): error attaching memory to clients pointer.\n");
		return NULL;
	}

	/* Selecting the first free client on the server client vector */
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i].id == 0) {
			/*Initializing the client*/
			clients[i].id = getpid();
			clients[i].semid = serv->semid;
			clients[i].shmidMessages = serv->shmidMessages;
			clients[i].offset = i * sizeof(shmMessage);
			clients[i].used = TRUE;
			break;
		}
	}
	if (i == MAX_CLIENTS) {
		fprintf(stderr, "connectToServer():the client vector is full.\n");
		return NULL;
	}


	down(serv->semid, SEM_CLI_TABLE);

	/* initializing the return client info */
	client = malloc(sizeof(struct clientCDT));
	if (client == NULL)
	{
		fprintf(stderr, "Malloc returned NULL in connectToServer.\n");
		return NULL;
	}

	client->semid = (clients[i]).semid;
	client->shmidMessages = (clients[i]).shmidMessages;
	client->offset = (clients[i]).offset;
	/* linking the shared memory where the client is supposed to write and read */
	client->memory = shmat(serv->shmidMessages, NULL,
			PERMS);

	if (client->memory == (void*) -1) {
		fprintf(
				stderr,
				"connectToServer(): error attaching memory to memory pointer.\n");
		free(client);
		return NULL;
	}

	/* Detaching client table */
	shmdt(clients);
	down(serv->semid, SEM_CLI_TABLE);

	return client;
}

clientADT getClient(serverADT server, pid_t id) {
	struct clientCDT * clients = (struct clientCDT *) server->clients;
	int i;
	clientADT client = NULL;

	if (up(server->semid, SEM_CLI_TABLE, TRUE) == -1) {
		fprintf(stderr, "getClient(): up operation failed\n");
		return NULL;
	}
	/* looking for client in client vector */
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i].used == FALSE) {
			/* Client is absent */
			down(server->semid, SEM_CLI_TABLE);
			return NULL;
		} else if (clients[i].id == id && clients[i].used == TRUE)
		{
			/*Client is present */
			client = malloc(sizeof(struct clientCDT));
			if (client == NULL)
			{
				fprintf(stderr,
						"getClient(): no enough memory space for client.\n");
				return NULL;
			}
			client->semid = (clients[i]).semid;
			client->shmidMessages = (clients[i]).shmidMessages;
			client->offset = (clients[i]).offset;
			client->memory = shmat(client->shmidMessages, NULL,
					PERMS);
			if (client->memory == (void*) -1) {
				fprintf(
						stderr,
						"getClient(): error attaching message memory to client");
				free(client);
				return NULL;
			}
			break;
		}
	}
	down(server->semid, SEM_CLI_TABLE);

	return client;
}

int sendMessage(clientADT client, message * msg, int flags) {
	int amtSent = 0;
	char * origin = (char *) (msg->message);
	void * destination;
	shmMessage * sending;

	if (msg == NULL || client == NULL) {
		fprintf(stderr,
				"sendMessage(): error in client communication parameters.\n");
		return -1;
	}

	sending = malloc(sizeof(shmMessage));

	if (sending == NULL) {
		fprintf(stderr, "sendMessage(): not enough memory to alloc message.\n");
		return -1;
	}

	if (up(client->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
		free(sending);
		fprintf(stderr, "sendMessage(): error initializing semaphore.\n");
		return -1;
	}
	/* variable used to write at the client's reserved position */
	destination = (void *)((int)client->memory + (int)client->offset);

	/*blocks the process if the message table is full*/
	while (((shmMessage*) destination)->isWritten == TRUE
			&& ((shmMessage*) destination)->quantity != 0) {
		down(client->semid, SEM_MEMORY);

		up(client->semid, SEM_MEMORY, TRUE);
	}

	/* Storing message*/
	sending->isWritten = TRUE;
	amtSent = (msg->size > MESG_SIZE) ? MESG_SIZE : msg->size;
	sending->quantity = amtSent;
	memcpy(sending->message, origin, amtSent);
	memcpy(destination, sending, sizeof(shmMessage));

	free(sending);

	down(client->semid, SEM_MEMORY);

	return amtSent;
}

int rcvMessage(clientADT client, message * msg, int flags) {
	int amtRcv = 0;
	void * origin;
	shmMessage * receiving;
	int ready;

	if (msg == NULL || client == NULL) {
		fprintf(stderr,
				"rcvMessage(): Error in client communication parameters.\n");
		return -2;
	}

	receiving = malloc(sizeof(shmMessage));

	if (receiving == NULL) {
		free(receiving);
		fprintf(stderr, "rcvMessage(): not enough memory to alloc message\n");
		return -2;
	}

	if (up(client->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
		free(receiving);
		return -2;
	}

	/* variable used to read from the client's reserved position */
	origin = (void *)((int)client->memory + (int)client->offset);

	memcpy(receiving, origin, sizeof(shmMessage));
	if (flags == IPC_NOWAIT) {
		if (receiving->isWritten == FALSE)
		{
			free(receiving);
			down(client->semid, SEM_MEMORY);
			return -1;
		}
	} else {
		down(client->semid, SEM_MEMORY);
		ready = FALSE;
		while (!ready) {
			if (up(client->semid, SEM_MEMORY, flags != IPC_NOWAIT) == -1) {
				free(receiving);
				return 0;
			}
			/* Reading message */
			memcpy(receiving, origin, sizeof(shmMessage));

			if (receiving->isWritten == TRUE)
			{
				ready = TRUE;
			} else {
				down(client->semid, SEM_MEMORY);
			}
		}
	}

	memcpy(msg->message, receiving->message, receiving->quantity);
	msg->size = receiving->quantity;
	amtRcv = receiving->quantity;

	free(receiving);
	((shmMessage*) origin)->isWritten = FALSE;
	((shmMessage*) origin)->quantity = 0;

	down(client->semid, SEM_MEMORY);
	return amtRcv;
}

int disconnectFromServer(clientADT client) {
	shmdt(client->memory);
	free(client);
	return 0;
}

int terminateServer(serverADT server) {
	shmdt(server->memory);
	shmdt(server->clients);
	shmctl(server->shmidClients, IPC_RMID, NULL);
	shmctl(server->shmidMessages, IPC_RMID, NULL);
	destroySem(server->semid);
	free(server);
	return 0;
}
