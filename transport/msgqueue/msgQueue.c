#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "../../include/api.h"
#include "../../include/structs.h"

typedef struct {
	long mtype;
	char mtext[500];
} msgQueue;


struct serverCDT {
	int queueID;
};

struct IPCCDT {
	int queueID;
	int pid;
};

static key_t queueKey = 0xBEEF0;
int queueid;

void reverse(char *string);
void itoa(int n, char *string);

servADT startServer() {
	if ((queueid = msgget(queueKey, 0666 | IPC_CREAT)) == -1)
		/*fatal("Error msgget message queue");*/
		printf("Error msgget");
	servADT serv = malloc(sizeof(servADT));
	serv->queueID = queueid;
	return serv;
}

comuADT connectToServer(servADT serv) {
	comuADT client = malloc( sizeof(comuADT));
	client->queueID = serv->queueID;
	client->pid = getpid();

	return client;
}

comuADT getClient(servADT serv, pid_t id) {
	comuADT client = malloc( sizeof(comuADT) );
	client->queueID = serv->queueID;
	client->pid = id;
	return client;
}

int sendMsg(comuADT comm, message * msg, int flags) {
	msgQueue aux;
	aux.mtype = comm->pid;
	strcpy(aux.mtext, msg->message);
	
	return msgsnd(comm->queueID, &aux,sizeof aux.mtext, flags);
}

int rcvMsg(comuADT comm, message * msg, int flags) {
	int ret;
	msgQueue aux;
	ret = msgrcv(comm->queueID, &aux, sizeof aux.mtext, comm->pid, flags);
	msg->message = malloc( ret + 1 );
	strcpy(msg->message, aux.mtext);
	((char *)msg->message)[ret] = 0;
	msg->size = strlen(msg->message);
	return ret;
}

int disconnectFromServer(comuADT comm, servADT server)
{
	return 0;
}

int endServer(servADT server)
{
	return 0;
}

int infoClient_comparePid(infoClient * ic1, infoClient * ic2)
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
