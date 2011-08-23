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
	int city;
	int companyID;
	int planeID;
	int medCount;
	medicine ** med;
	med = malloc ( sizeof(medicine *) * 2);
	med[0] = malloc( sizeof(medicine) );
	med[1] = malloc( sizeof(medicine) );
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
		rcvClient = getClient(server, getppid());
		/*	while (true) {*/
		raise(SIGSTOP);
		n = rcvPackage(&city, &med, rcvClient, &companyID, &planeID );
		strcpy(apimsg.message,(char *)wrappMedicine(city, med, companyID, planeID, medCount) );
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
			rcvClient = getClient(server, getppid());
			/*	while (true) {*/
			raise(SIGSTOP);
			n = rcvPackage(&city, &med, rcvClient, &companyID, &planeID );
			strcpy(apimsg.message,(char *)wrappMedicine(city, med, companyID, planeID, medCount) );
			printf("Second son: I've received %s !\n", (char *)apimsg.message);
			printf("%d chars\n", n);
			_exit(0);
			/*	}*/
			break;

		default:
			/*father*/
			client = connectToServer(server);
			city = 4;
			companyID = 5;
			planeID = 6;
			medCount = 2;
			med[0]->name = "merca";
			med[0]->quantity = 4;
			med[1]->name = "cacona";
			med[1]->quantity = 3;



			sleep(2);
			sendPackage(city,med,client, companyID, planeID, medCount);
			kill(pids[1], SIGCONT);
			/*pid = wait(&status);*/
			sleep(2);
			city = 8;
			companyID = 9;
			sendPackage(city,med,client, companyID, planeID, medCount);
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
