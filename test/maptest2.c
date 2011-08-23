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
	comuADT client, rcvClient;
	servADT server;
	server = startServer();

	switch (pids[1] = fork()) {
	case -1:
		fatal("error in first son conception");
		break;
	case 0:
		/* first son */
		printf("I am the first son\n");
		sleep(1);
		client = connectToServer(server);
		rcvClient = getClient(server, getpid());
		/*	while (true) {*/
		raise(SIGSTOP);
		n = rcvMsg(rcvClient , &apimsg, 0);
		printf("First son: I've received %s !\n", (char *)apimsg.message);
		printf("%d chars\n", n);

		/*	}*/
		_exit(0);
		break;
	default:
		printf("I am tha father\n");
		switch (pids[2] = fork()) {
		case -1:
			fatal("error in second son conception");
			break;
		case 0:
			/* second son */
			printf("I am the second son\n");
			client = connectToServer(server);
			rcvClient = getClient(server, getpid());
			/*	while (true) {*/
			raise(SIGSTOP);
			n = rcvMsg(rcvClient , &apimsg, 0);
			printf("Second son: I've received %s !\n", (char *)apimsg.message);
			printf("%d chars\n", n);
			_exit(0);
			/*	}*/
			break;

		default:
			/*father*/
			client = connectToServer(server);
			strcpy(apimsg.message, "you are a loser");
			apimsg.size = sizeof( apimsg.message);
			sleep(2);
			client->pid = pids[1];
			sendMsg(client, &apimsg, 0);
			kill(pids[1], SIGCONT);
			/*pid = wait(&status);*/
			sleep(2);
			strcpy(apimsg.message, "you are the chosen one");
			client->pid = pids[2];
			sendMsg(client, &apimsg, 0);
			kill(pids[2], SIGCONT);
			/*pid = wait(&status);*/
			sleep(2);
			printf("father running\n");

			/*kills running processes before quitting*/
			kill (pids[1], SIGTERM);
			kill (pids[2], SIGTERM);
			break;
		}
		break;
		return 0;
	}
}

void fatal(char *s) {
	perror(s);
	exit(1);
}
