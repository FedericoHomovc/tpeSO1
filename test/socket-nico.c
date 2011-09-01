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
		//printf("address:%d\n",client.sin_addr.s_addr);
		//printf("family:%d\n",client.sin_family);
		//printf("port:%d\n",client.sin_port);
		//printf("mipides:%d\n",getpid()); 
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
			if (comm == NULL){
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
			printf("Thread >> El pid que me quedo para manejarme es:%d\n",*(int *)(msg.message));
			icp->id = *(int *)(msg.message);
			
			vArray_insertAtEnd(server->infoClientArray, icp);

			//pthread_t temp;
			//pthread_create(&temp, NULL, listeningSocket, (void *) icp);
			//pthread_detach(temp);
		}else{
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
	/*do{
		int bindReturn = bind(server->sockFd, (struct sockaddr *) &(server->socketAddress), sizeof(struct sockaddr_in));
		if( bindReturn == -1){
			server->socketAddress.sin_port++;
		}
	}while(bindReturn == -1);
	
	do{
		listenReturn = listen(server->sockFd,5);
	}while(listenReturn == -1);*/
	int bindRet = -1;
	while (bindRet == -1)
	{
		server->socketAddress.sin_port++;
		bindRet = bind(server->sockFd, (struct sockaddr *) &(server->socketAddress),
				sizeof(struct sockaddr_in));
	}

	listen(server->sockFd, 5);

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
	printf("ConnectToServer >> Cuando mando mando el pid:%d\n",getpid());
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
	printf("ASDADADSAASDASAD");
	if (server == NULL || server->infoClientArray == NULL)
		return NULL;
	infoClient ic;
	ic.id = id;
	ic.comm = NULL;
	printf("Antes de usar el Varray_search, el pid es %d\n", id);
	infoClient * icp = ((infoClient *) (vArray_search(server->infoClientArray,
			(int(*)(void *, void *)) infoClient_comparePid, &ic)));
	printf("Despues de usar el Varray_Search\n");
	if (icp != NULL){
		printf("Encontre un cliente para devolver y Su pid es: %d\n", icp->id);
		return icp->comm;
	}else
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

pid_t pids[10];
int status;

int main() {
	int pid;
	int qid;
	int n;
	message apimsg;
	comuADT client, rcvClient, sndClient;
	servADT server;
	server = startServer();

	/*initialization of all sons*/
	int k = 1;
	int sonsCount = 3;
	int turns = 2;
	int i = 0;
	
	while (k < sonsCount) {
		switch (pids[k] = fork()) {
		case -1:
			fatal("error in son conception");
			break;
		case 0:
			sleep(1);
			printf("I am son No %d\n", k);
			int rndtime;
			unsigned int iseed = (unsigned int)time(NULL);
			srand (iseed);
			
			client = connectToServer(server);
			//comuADT nuevo_clientes;
			while (1) {
				raise(SIGSTOP);
				n = rcvMsg(client, &apimsg, 0);
				printf("Son %d: I've received %s !\n", k,
						(char *) apimsg.message);
				//malloc(1000);				
				printf("Hola como te va %d\n",5);
				printf("me duermo\n");
				sleep(2);
				printf("Me levanto y mando..\n");
				
				getClient(server, getppid());
				//printf("Hijo: Mi padre tiene el sockfd %d\n", sndClient->sockFd);
				//printf("Hijo: El pid del padre que tengo es:%d\n", getppid());
				//sprintf(apimsg.message, "send U this: %d", atoi(apimsg.message) );
				//sendMsg(sndClient, &apimsg, 0);				
				//printf("%d charsSSSS\n", n);
				
				/*rndtime = (int)(rand ()*3);
				sleep(rndtime);*/
				
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
	printf("Padre: Mi pid es %d\n",getpid());
	client = connectToServer(server);
	//sleep(2);
	//printf("me conecte\n");
	while (turns) {
		/* does the broadcast */
		printf("\nbroadcasting\n");
		while (i < sonsCount) {
			itoa(i, buf);
			/*strncpy(string, string,strlen(string)-1 );
			 strcat(string, buf);*/
		//printf("%s\n", buf);
			/*apimsg.message = "hijito";*/
			apimsg.message = malloc(sizeof buf);
			strcpy(apimsg.message, buf);
			apimsg.size = strlen(apimsg.message);
			sndClient = getClient(server, pids[i]);/*si se descomenta esto no anda*/
			printf("Mande %s\n",(char *)apimsg.message);
			sendMsg(sndClient, &apimsg, 0);
			kill(pids[i], SIGCONT);
			i++;
			sleep(1);
		}
		i = 1;
		printf("\nreceiving messages\n");
		/* recieves the messages */
		while (i < sonsCount) {
			printf("Antes\n");
			sleep(2);
			printf("Padre: Mi sockFd es: %d\n", client->sockFd);
			n = rcvMsg(client, &apimsg, 1);
			printf("Despues\n");
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
