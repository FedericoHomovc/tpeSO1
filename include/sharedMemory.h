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
/* The maximum amount of clients that the IPC can handle */
#define MAX_CLIENTS 400
#define SIZE_CLIST ((size_t)(sizeof(comuADT) * (MAX_CLIENTS + 1)))
#define SIZE ((size_t)(sizeof(sharedMemoryMessage)*MAX_CLIENTS*2))

#define SEM_CLI_TABLE 0
#define SEM_MEMORY 1


/*
 * Name: cleanUP
 * Receives: void * mem, int bytes
 * Returns: void
 * Description: This function zeroes out a certain amount of memory,
 * starting from mem until mem + bytes - 1.
 */
static void cleanUP(void * mem, int bytes);

/* Starts a Server */
servADT startServer();

/* Connects a client to a given server */
comuADT connectToServer(servADT serv);

/* Gets a client from the server side */
comuADT getClient(servADT server, pid_t id);

int sendMsg(comuADT comm, message * msg, int flags);

/* Receives a message through comm */
int rcvMsg(comuADT comm, message * msg, int flags);

/* Disconnects a client from a server */
int disconnectFromServer(comuADT comm, servADT server);

/* Ends a server */
int endServer(servADT server);

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
 * - shmidMemory: The shared memory ID to be able to access the message table.
 * - offset: The offset (of the message table) assigned for communication. 
 * - memory: The pointer to the shared memory already attached. Is only valid
 * to the process who obtained this struct from connectToServer or getClient.
 */
struct IPCCDT
{
	pid_t id;
    int used;
    int semid;
    int shmidMemory;
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
 * - shmidMemory: The shared memory ID of the messages table.
 * - clients: The pointer to the shared memory client table already attached.
 * It has no sense for a client, only useful on the server side.
 * - memory: The pointer to the shared memory message table already attached.
 * It has no sense for a client, only useful on the server side.
 * - maxClients: The maximum amount of clients that may connect through one 
 * server.
 */
struct serverCDT
{
    int semid;
    int shmidClients;
    int shmidMemory;
    /* The following two parameters, are LOCAL to the server
     * They have no sense on the client side */
    void * clients;
    void * memory;
    int maxClients;
};



/*
 * sharedMemoryMessage
 * Description: Represents a shared memory message, the messages that are
 * stored on the message table.
 * Fields:
 * - amount: Specifies the length of the message.
 * - message: The message itself.
 */
typedef struct sharedMemoryMessage
{
    int isWritten;
    int amount;
    char message[MESG_SIZE];
} sharedMemoryMessage;

