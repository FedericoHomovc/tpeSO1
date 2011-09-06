/*
 * Includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <ctype.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>
#include <signal.h>
#include "../../include/api.h"
#include "../../include/varray.h"

/*
 * Structs
 */

/* Struct underlying comuADT, have a sockFd Integer that represent the file
Socket descriptor */
struct IPCCDT
{
	int sockFd;
};

/* Struct underlying servADT */
struct serverCDT
{
	/* Socket address of the server. */
	struct sockaddr_in socketAddress;

	/* Socket file descriptor for the server. */
	int sockFd;

	vArray infoClientArray;
	pthread_t listeningThread;
};

/* Initial size for the infoClient vArray */
#define ICA_INITIAL_SIZE 100

/*
 * Static functions
 */

/*
 * Name: listeningSocket
 * Receives: void * ic
 * Returns: void *
 * Description: The function that each of the individual threads that
 * take care of finishing each of the new clients' connections runs.
 * Receives an infoClient * whose comm_t has already been initialized,
 * and basically waits for the client to send its pid to store it
 * on the main server table.
 */
static void * listeningSocket(void * ic)
{
	infoClient * icp = (infoClient *) ic;
	message msg =
	{ sizeof(pid_t), &(icp->id) };
	int i;
	while ((i = rcvMsg(icp->comm, &msg, 0)) < sizeof(pid_t))
	{
		printf("rcvMsg fail, value was %d\n", i);
	}
	if (FALSE)
	{
		/* Now send a message */
		char c;
		message m =
		{ sizeof(char), &c };
		sendMsg(icp->comm, &m, 0);
	}
	pthread_exit(NULL);
}

/*
 * Name: listeningConnection
 * Receives: void * server
 * Returns: void *
 * Description: The function used by the listening thread in the server.
 * Is in charge of listening for new clients and accepting their connection
 * requests. Adds them to the client list, but also creates a new thread
 * that waits for them to send their pid to finish completing the infoClient.
 */
static void * listeningConnection(void * serv)
{
	servADT server = (servADT) serv;

	/* First initialize the table! */
	server->infoClientArray = vArray_init(ICA_INITIAL_SIZE);
	if (server->infoClientArray == NULL)
	{
		/* Major problem */
		fprintf(stderr, "Not enough memory to create array!\n");
		exit(-1);
	}

	/*comuADT commTemp = malloc(sizeof(struct IPCCDT));
	if (commTemp == NULL)
	{
		
		fprintf(stderr, "Not enough memory to create comm_t!\n");
		exit(-1);
	}

	commTemp->sockFd = server->sockFd;*/

	/* Now listen */
	while (1)
	{
		struct sockaddr_in client;
		int var;
		/* Accept any incoming connections */
		int newSockFd = accept(server->sockFd,(struct sockaddr *)&client, &var);
		printf("address:%d\n",client.sin_addr.s_addr);
		printf("family:%d\n",client.sin_family);
		printf("port:%d\n",client.sin_port);
		printf("mipides:%d\n",getpid()); 
		if (newSockFd != -1)
		{
			/*
			 * newSockFd has the new socket file descriptor,
			 * we need to know what PID it is.
			 * First, insert it into the array.
			 */
			infoClient * icp = malloc(sizeof(infoClient));

			if (icp == NULL)
			{
				/* Major problem */
				fprintf(stderr, "Not enough memory to create infoClient!\n");
				exit(-1);
			}
			icp->id = getpid();

			comuADT comm = malloc(sizeof(comuADT));
			if (comm == NULL)
			{
				/* Major problem */
				fprintf(stderr, "Not enough memory to create comm_t!\n");
				exit(-1);
			}
			comm->sockFd = newSockFd;
			icp->comm = comm;

			message msg = { sizeof(pid_t), &(icp->id) };
			int i;
			while ((i = rcvMsg(icp->comm, &msg, 0)) < sizeof(pid_t))
			{
				printf("rcvMsg fail, value was %d\n", i);
			}
			printf("El valor de lo que me mandaron cuando conecto es:%d\n",*(int *)(msg.message));
			icp->id = *(int *)(msg.message);
			
			vArray_insertAtEnd(server->infoClientArray, icp);

			//pthread_t temp;
			//pthread_create(&temp, NULL, listeningSocket, (void *) icp);
			//pthread_detach(temp);
		}
		else
		{
			fprintf(stderr, "Accept call failed\n");
		}
	}

	return NULL;
}


/* Starts a server */

servADT startServer()
{
	int bindReturn = 0;
	int listenReturn = 0;
	
	servADT server = malloc(sizeof(struct serverCDT));
	if (server == NULL)
		return NULL;

	if ( (server->sockFd = socket (AF_INET, SOCK_STREAM,0) ) == -1)
	{
		perror("Socket call failed");
		free(server);
		return NULL;
	}

	/*Set Localhost*/
	server->socketAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	/*Set internet address family */	
	server->socketAddress.sin_family = AF_INET;
	/*Set port number*/
	server->socketAddress.sin_port = 11000;
	/*Set Clients in NULL*/
	server->infoClientArray = NULL;

	/*Start and complete binding*/
	do{
		int bindReturn = bind(server->sockFd, (struct sockaddr *) &(server->socketAddress), sizeof(struct sockaddr_in));
		if( bindReturn == -1){
			server->socketAddress.sin_port++;
		}
	}while(bindReturn == -1);
	
	/*Start listening*/
	do{
		listenReturn = listen(server->sockFd,5);
	}while(listenReturn == -1);

	//TODO: ACA HAY QUE PONER ALGO A ESCUCHAR ALL EL FUCKING TIME!
	pthread_create(&(server->listeningThread), NULL, listeningConnection,
			(void *) server);

	pthread_detach(server->listeningThread);	

	return server;
}

/* Connects to a server */
comuADT connectToServer(servADT server)
{

	comuADT comm = (comuADT) malloc ( sizeof( struct IPCCDT));
	if ( comm == NULL )
		return NULL;
	
	comm->sockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (comm->sockFd == -1)
	{
		perror("Socket has failed\n");		
		free(comm);
		return NULL;
	}

	/*TODO: Esto esta al dope ...*/
	
	struct sockaddr_in socketAddress;
	socketAddress.sin_addr.s_addr = INADDR_ANY;
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = 21000; 
/*
int bindRet = -1;
	while (bindRet == -1)
	{
		socketAddress.sin_port++;
		bindRet = bind(server->sockFd, (struct sockaddr *) &socketAddress,
				sizeof(struct sockaddr_in));
	}
	printf("Pase por aca 2\n",comm->sockFd);*/

	/*Connect to server*//*
	if( connect(server->sockFd, (struct sockaddr *)&(server->socketAddress),
				sizeof(struct sockaddr_in)) == -1){
		perror("Connection has failed\n");
		return NULL;
	}*/
	int bindRet = -1;
	while (bindRet == -1)
	{
		socketAddress.sin_port++;
		bindRet = bind(comm->sockFd, (struct sockaddr *) &socketAddress,
				sizeof(struct sockaddr_in));
	}

	int connectRet = -1;
	while (connectRet == -1)
		connectRet = connect(comm->sockFd,
				(struct sockaddr *) &(server->socketAddress),
				sizeof(struct sockaddr_in));

	/* We are connected. Must send PID */
	pid_t pid;
	pid = getpid();
	message msg = { sizeof(pid_t), &pid };
	while (sendMsg(comm, &msg, 0) < sizeof(pid_t))
		;
	printf("Cuando mando mando el pid:%d\n",getpid());
	/* We sent the PID to the server. Return */
	/*if (FALSE)
	{
		But first, wait until the server sends me a byte 
		char c;
		message m =
		{ sizeof(char), &c };
		rcvMsg(comm, &m, 0);
	}*/
	//printf("El valor del socket es:%d\n",comm->sockFd);
	return comm;
}

/* Gets a client from the client list */
comuADT getClient(servADT server, pid_t id)
{
	if (server == NULL || server->infoClientArray == NULL)
		return NULL;
	infoClient ic;
	ic.id = id;
	ic.comm = NULL;
	infoClient * icp = ((infoClient *) (vArray_search(server->infoClientArray,
			(int(*)(void *, void *)) infoClient_comparePid, &ic)));
	if (icp != NULL)
		return icp->comm;
	else
		return NULL;

}

/* Sends a message */
int sendMsg(comuADT comm, message * msg, int flags)
{
	int ret = 0;
	ret = send(comm->sockFd, msg->message, msg->size, 0);
	return ret;
	
	/*int ret = send(comm->sockFd, msg->message, msg->size, 0);
	return ret;*/
}

/* Receives a message */
int rcvMsg(comuADT comm, message *msg, int flags)
{
	int ret = recv(comm->sockFd, msg->message, msg->size,
			(flags & IPC_NOWAIT) == 0 ? MSG_WAITALL : 0);
	return ret;
}

/* Disconnects a client from the server */
int disconnectFromServer(comuADT comm, servADT server)
{
	close(comm->sockFd);
	free(comm);
	return 0;
}

/* Ends the server */
int endServer(servADT server)
{
	pthread_cancel(server->listeningThread);
	close(server->sockFd);
	int i;
	for (i = 0; i < vArray_getSize(server->infoClientArray); i++)
	{
		infoClient * icp = vArray_getAt(server->infoClientArray, i);
		if (icp != NULL)
		{
			close(icp->comm->sockFd);
			free(icp->comm);
			free(icp);
		}
	}
	vArray_destroy(server->infoClientArray);
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


static key_t qkey = 0xBEEF0;
typedef enum {
	false, true
} bool;

void fatal(char *s);
void itoa(int n, char *string);
void reverse(char *string);

void fatal(char *s) {
	perror(s);
	exit(1);
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



/*void main (){
	printf("Esto Arranco\n");
	serverADT

}*/
/*main(){

printf("Esto funciona..\n\n");

if( fork() == 0){

servADT server = startServer();
printf("Creo el server\n");
}else{
comuADT cliente1 = connectToServer(server);
printf("Cliente 1 enganchado\n");

comuADT cliente2 = connectToServer(server);
printf("Cliente 2 enganchado\n\n");

message mensaje = {10,"holaholac"};
message * msg = &mensaje;
message * rcvmsg;
//printf("Longitud:%ld\nMensaje:%s\n",rcvmsg->size,(char *)rcvmsg->message);

printf("\nMandando...\n");
printf("Longitud:%ld\nMensaje:%s\n",msg->size,(char *)msg->message);
sendMsg(cliente1,msg,0);
printf("Mensaje Mandado!\n\n");

while( rcvMsg(cliente2,rcvmsg,0) > 0){
sendMsg(cliente1,msg,0);
}

printf("Recibiendo mensaje...\n");
printf("Mensaje recivido\nEl mensaje es:\n");
printf("Longitud:%ld\nMensaje:%s\n",rcvmsg->size,(char *)rcvmsg->message);

//endServer(server);
//printf("Hizo pelota el server\n");

printf("Esto termino\n");
}
}*/
