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

#include "maptest.h"
#include "api.h"
#include "structs.h"

static key_t qkey = 0xBEEF0;
typedef enum {
	false, true
} bool;

void fatal(char *s);

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
			printf("I am son No %d\n", k);

			client = connectToServer(server);
//			raise(SIGSTOP);
			rcvClient = getClient(server, getpid());

			while (true)
				if ((n = rcvMsg(rcvClient, &apimsg, 0)) > 0) {
					printf("Son: I've received %s !\n", (char *) apimsg.message);
					printf("%d chars\n", n);
				}

//			printf("Son: I've received %s !\n", (char *) apimsg.message);
//			printf("%d chars\n", n);
			_exit(0);
			break;
		}
		k++;
	}
	sleep(2);

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
	strcpy(string, "you are son ");
	printf("I am tha father and lord\n");
	client = connectToServer(server);
	while (turns) {
		/* does the broadcast */
		printf("broadcasting\n");
		while (i < sonsCount) {
			itoa(i, buf, 5);
			strcat(string, buf);
			printf("%s\n", buf);
			/*strcpy(apimsg.message, buf);*/
			apimsg.message = "hijito";
			apimsg.size = sizeof(apimsg.message);
			sndClient = getClient(server, pids[i]);
			sendMsg(sndClient, &apimsg, 0);
//			kill(pids[i], SIGCONT);
			i++;
			sleep(1);
		}
		i = 1;
		/* recieves the messages */
		/*while (i <= sonsCount){

		 }*/
//		sleep(2);
		turns--;
	}
	/*kills running processes before quitting*/
	int q = 1;
	while (q < sonsCount) {
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
