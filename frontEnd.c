/***
***
***		frontEnd.c
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

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

/***		Project Includes		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/frontEnd.h"
#include "./include/api.h"
#include "./include/fifo.h"


/*
 * global variables
 */
comuADT * clients;
servADT server;

int
main(int argc, char * argv[])
{
	int status, k = 0;
	pid_t pid, pid2;
	int * pids;

	server = startServer();
	clients = malloc(sizeof(struct IPCCDT)*(argc)); /*poner en el back*/
	clients[0] = connectToServer(server);  /*main client*/

	if (argc<=2)
	{
		printf("Invalid arguments\n");
		return 1;
	}

	switch( pid = fork() ){
		case -1:
			perror("creating map");
			exit(1);
		case 0:
			printf("empieza el mapa\n");
			execl("map", "map", argv[1], (char *) 0);
			_exit(0);
		default:
			switch(pid2 = fork()){
				case -1:
					perror("creating IO");
					exit(1);
				case 0:
					printf("empieza IO\n");
					execl("io", "io", (char *) 0);
					_exit(0);
				default:
					pids = malloc(sizeof(int) * (argc-2));
					while(k < argc-2)
					{
						switch( pids[k] = fork() ){
							case -1:
								perror("creating company");
							case 0:
								printf("empieza la company %d\n", k);
								execl("company", "company", argv[k+2], (char *) 0);
								_exit(0);
						}
						k++;
					}
			}
			sleep(2);
			while (waitpid(pid, &status, WNOHANG) == 0)
			{
				printf("waiting for map to end...\n");
				sleep(1);
			}
			if( WIFEXITED(status) )
			{
				printf("termino el mapa\n");
				kill(pid2, SIGTERM);
				kill(pids[0], SIGTERM);
				endServer(server);
			}
	}

	/*wait(&rv);
	printf("termina el mapa\n");*/

/*	startSimulation();*/
/*	freeResources();*/

	return 0;
}
