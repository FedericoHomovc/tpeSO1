/***
***
***				transport.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

#ifndef TRANSPORT_H_
#define TRANSPORT_H_

/***		System includes		***/
#include <sys/types.h>


/***		Module Defines		***/
#define TRUE 1
#define FALSE 0
#define MSG_SIZE 1630

/***		Structs 		***/
typedef struct serverCDT * serverADT;
typedef struct clientCDT * clientADT;

/*
 * struct IPCMessage
 *
 * This structure is used to send and receive messages for each
 * different IPC. The message is stored as a void pointer and the
 * message size is also stored so no data is lost.
 */
typedef struct IPCMessage
{
	long size;
	void * message;
} message;


/*
 * struct infoClient
 *
 * This structure stores the basic information of the client. It is
 * used by the server to store the clients and search them by their PID.
 */
typedef struct
{
	pid_t id;
	clientADT client;
} infoClient;

/***		Functions		***/

/*
 * function createServer
 *
 * Creates a new server, allocating memory and initializing all the 
 * underlaying variables. Returns the new server created or NULL if 
 * an error occured.
 */
serverADT createServer();

/*
 * function connectToServer
 *
 * Creates a new client and connects it to the server. Depending on the
 * implementation it opens new files or reserves the suficient memory
 * so the communcation can be established. The client is stored within
 * the server's clients so it can be accessed by other clients. Returns
 * the new client created or NULL if an error occured.
 *
 * @serv: server to connect with.
 */
clientADT connectToServer(serverADT serv);

/*
 * function getClient
 *
 * Returns the client connected to the server which pid equals the pid given
 * as a parameter, or NULL if the client is not found.
 *
 * @serv: Server where to search for the client.
 * @id: Process ID of the client required.
 */
clientADT getClient(serverADT serv, pid_t id);

/*
 * function sendMessage
 *
 * Sends the message in the structure @msg to the client given as a parameter.
 * Flags are used to set the calling as blocking or not.
 *
 * @client: Client to send the message to.
 * @msg: Message to be sent.
 * @flags: Flags to set if the function should be blocking or not.
 */
int sendMessage(clientADT client, message * msg, int flags);

/*
 * function rcvMessage
 *
 * Receives a message for the client given as a parameter and stores the message
 * in the argument @msg given.
 *
 * @client: Client that receives the message.
 * @msg: Structure where the message is stored.
 * @flags: Flags to set if the function should be blocking or not.
 */
int rcvMessage(clientADT client, message * msg, int flags);

/*
 * function disconnectFromServer
 *
 * Disconnects the given client from the server, so it no longer can send or receive
 * messages. Frees any memory allocated by the client.
 *
 * @client: Client to be disconnected from server.
 */
int disconnectFromServer(clientADT client);

/*
 * function terminateServer
 *
 * Terminates the server given, freeing all the memory allocated and closing all the 
 * files that the server opened for communication.
 *
 * @server: Server to be terminated.
 */
int terminateServer(serverADT server);

#endif
