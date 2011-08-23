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
	msgQueue message;

	switch (pids[1] = fork()) {
	case -1:
		fatal("error in first son conception");
		break;
	case 0:
		/* first son */
		printf("I am the first son\n");
		sleep(1);

		qid = msgget(qkey, 0666 | IPC_CREAT); /*obtains the handler for the queue*/

		/*	while (true) {*/
		raise(SIGSTOP);
		n = msgrcv(qid, &message, sizeof message.mtext, getppid(), 0);
		printf("First son: I've received %s !\n", message.mtext);
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
			qid = msgget(qkey, 0666 | IPC_CREAT); /*obtains the handler for the queue*/
			msgQueue message2;
			/*	while (true) {*/
			raise(SIGSTOP);
			n = msgrcv(qid, &message, sizeof message.mtext, getppid(), 0);
			printf("Second son: I've received %s !\n", message.mtext);
			printf("%d chars\n", n);
			_exit(0);
			/*	}*/
			break;

		default:
			/*father*/
			qid = msgget(qkey, 0666 | IPC_CREAT); /*obtains the handler for the queue*/
			msgQueue message;
			strcpy(message.mtext, "you are a loser");
			message.mtype = getpid();
			sleep(2);
			msgsnd(qid, &message, sizeof message.mtext, 0);
			kill(pids[1], SIGCONT);
			/*pid = wait(&status);*/
			sleep(2);
			strcpy(message.mtext, "you are the chosen one");

			msgsnd(qid, &message, sizeof message.mtext, 0);
			kill(pids[2], SIGCONT);
		/*	pid = wait(&status);*/
			sleep(2);
			printf("father running\n");
			return 0;
			break;
		}
		break;
	}
}

void fatal(char *s) {
	perror(s);
	exit(1);
}
