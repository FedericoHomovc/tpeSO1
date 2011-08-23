/***		System includes		***/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <error.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#include "maptest.h"
#include "api.h"
#include "structs.h"
#include "marshalling.h"

static key_t qkey = 0xBEEF0;
typedef enum {
	false, true
} bool;

void fatal(char *s);
void itoa(int n, char *string);

pid_t pids[10];
int status;

int main() {
	message apimsg, anothermsg;
	comuADT client, rcvClient, sndClient;
	servADT server;
	int k = 1, i = 0, n;
	int sonsCount = 2;
	int turns = 2;

	server = startServer();
	client = connectToServer(server);

	/*initialization of all sons*/
	while (k < sonsCount) {
		switch (pids[k] = fork()) {
		case -1:
			fatal("error in son conception");
			break;
		case 0:
			printf("I am son No %d\n", k);
			/*int rndtime;
			unsigned int iseed = (unsigned int)time(NULL);
			srand (iseed);*/
			client = connectToServer(server);

			while (1) {
				raise(SIGSTOP);
				apimsg.message = malloc(20);
				anothermsg.message = malloc(20);
				n = rcvMsg(client, &apimsg, 0);
				printf("Son %d: I've received %s !\n", k, (char *) apimsg.message);
				printf("%d chars\n", n);
				sndClient = getClient(server, getppid());
				sprintf(anothermsg.message, "send U this: %d", atoi(apimsg.message) );
				anothermsg.size = 20;
				printf("apimsg values: %s, %ld\n", (char*)anothermsg.message, anothermsg.size);
				printf("send from child: %d\n", sendMsg(client, &anothermsg, 0));
				printf("prueba %ld\n", anothermsg.size);
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
	/*client = connectToServer(server);*/
	while (turns) {
		/* does the broadcast */
		printf("\nbroadcasting\n");
		while (i < sonsCount) {
			itoa(i, buf);
			printf("%s\n", buf);
			apimsg.message = malloc(20);
			strcpy(apimsg.message, buf);
			apimsg.size = 20;
			sndClient = getClient(server, pids[i]);
			sendMsg(sndClient, &apimsg, 0);
			kill(pids[i], SIGCONT);
			i++;
			sleep(1);
		}
		i = 1;
		printf("\nreceiving messages\n");
		/* recieves the messages */
		while (i < sonsCount) {
			n = rcvMsg(client, &apimsg, /*IPC_NOWAIT*/ 0);
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
