/***
***
***		shm.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

/*
 * Symbolic constants
 */
#define KEY_1 (key_t)0x1000
#define KEY_2 (key_t)0x10000
/* The maximum message size in bytes */
#define MESG_SIZE 4096
/* The maximum quantity of clients that the IPC can handle */
#define MAX_CLIENTS 400
#define SIZE_CLI_VEC ((size_t)(sizeof(clientADT) * (MAX_CLIENTS + 1)))
#define SIZE ((size_t)(sizeof(shmMessage)*MAX_CLIENTS*2))

#define SEM_CLI_TABLE 0
#define SEM_MEMORY 1


/*
 * Name: cleanUP
 * Receives: void * mem, int bytes
 * Returns: void
 * Description: This function zeroes out a certain quantity of memory,
 * starting from mem until mem + bytes - 1.
 */
static void cleanUP(void * mem, int bytes);

/* Starts a Server */
serverADT startServer();

/* Connects a client to a given server */
clientADT connectToServer(serverADT serv);

/* Gets a client from the server side */
clientADT getClient(serverADT server, pid_t id);

int sendMsg(clientADT comm, message * msg, int flags);

/* Receives a message through comm */
int rcvMsg(clientADT comm, message * msg, int flags);

/* Disconnects a client from a server */
int disconnectFromServer(clientADT comm, serverADT server);

/* Ends a server */
int endServer(serverADT server);

/* Receives a string and stocks in it the reversed representation of it. */
void reverse(char *string);

/* Stores in string the string-representation of n. */
void itoa(int n, char *string);


/*
 * Structs
 */

/*
 * comuCDT
 * Description: This is the concrete data type for the ADT comm_t. It holds
 * the necessary information for a connection to take place once a server
 * is started.
 * Fields:
 * - used: Flag to set if this position in the client table is empty or not.
 * Not relevant for communication, but must be present for initialization 
 * purposes. 
 * - semid: The semaphore ID to be able to mutually exclude access to the 
 * message table. 
 * - shmidMessages: The shared memory ID to be able to access the message table.
 * - offset: The offset (of the message table) assigned for communication. 
 * - memory: The pointer to the shared memory already attached. Is only valid
 * to the process who obtained this struct from connectToServer or getClient.
 */
struct clientCDT
{
	pid_t id;
    int used;
    int semid;
    int shmidMessages;
    int offset;
    void * memory;
};

/*
 * servCDT
 * Description: This is the concrete data type for the ADT serv_t, it holds 
 * information to connect to a server or end a server. 
 * Fields:
 * - semid: The semaphore ID to be able to mutually exclude access to the 
 * message/clients table.
 * - shmidClients: The shared memory ID of the client's table. 
 * - shmidMessages: The shared memory ID of the messages table.
 * - clients: The pointer to the shared memory client table already attached.
 * It has no sense for a client, only useful on the server side.
 * - memory: The pointer to the shared memory message table already attached.
 * It has no sense for a client, only useful on the server side.
 * - maxClients: The maximum quantity of clients that may connect through one
 * server.
 */
struct serverCDT
{
    int semid;
    int shmidClients;
    int shmidMessages;
    void * clients;
    void * memory;
    int maxClients;
};



/*
 * shmMessage
 * Description: Represents a shared memory message, the messages that are
 * stored on the message table.
 * Fields:
 * - quantity: Specifies the length of the message.
 * - message: The message itself.
 */
typedef struct shmMessage
{
    int isWritten;
    int quantity;
    char message[MESG_SIZE];
} shmMessage;

