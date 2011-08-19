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
#include "./include/api.h"
#include "./include/structs.h"
#include "./include/backEnd.h"

int mapFunc(processData * pdata, char * fileName , int argc);
int companyFunc(processData * pdata, char * fileName , int companyID);
int rcvPackage(int * city, medicine ** med, comuADT client, int * companyID, int * planeID );


int
main(int argc, char * argv[])
{
	processData * pdata;
	int status, k, rec;;
	pid_t * pids;
	servADT server;
	comuADT * clients;

	pdata = malloc( sizeof(  processData ) );
	server = startServer();
	pdata->server = server;
	pdata->pid = getpid();
	pdata->name = malloc( sizeof(char *) * 20); /* tamaño del string */
	
	pids = malloc(sizeof(pid_t) * (argc)); /*poner en el back*/
	clients = malloc( sizeof(comuADT *)*(argc+1) ); /*poner en el back*/

	if (argc<=2)
	{
		printf("Invalid arguments\n");
		return 1;
	}

	switch( pids[0] = fork() ){
		case -1:
			perror("creating map");
			exit(1);
		case 0:
			printf("empieza el mapa\n");
			mapFunc(pdata, argv[1], argc);
			/*execl("map", "map", argv[1], (char *) 0);*/
			_exit(0);
		default:
			switch(pids[1] = fork()){
				case -1:
					perror("creating IO");
					exit(1);
				case 0:
					printf("empieza IO\n");
					ioFunc(pdata);
					/*execl("io", "io", (char *) 0);*/
					_exit(0);
				default:
					k = 2;
					while(k < argc)
					{
						switch( pids[k] = fork() ){
							case -1:
								perror("creating company");
							case 0:
								printf("empieza la company %d\n", k-2);
								companyFunc(pdata, argv[k] , k-2);
								/*execl("company", "company", argv[k+2], (char *) 0);*/
								_exit(0);
						}
						k++;
					}
			}
			sleep(1);

			clients[0] = connectToServer(server);  /*main client*/
			clients[1] = getClient(server, pids[0]);/*mapClient*/
			clients[2] = getClient(server, pids[1]);/*ioClient*/
			for( k=3; k<argc+1; k++)
			{
				clients[k] = getClient(server, pids[k-1]);
			}

			/*---------TESTING----------*/
			int city;
			int companyID;
			int planeID;
			medicine * med;

			printf("%d\n", rcvPackage(&city, &med, clients[3], &companyID, &planeID));
			/*printf("%s -- %d -- %d -- %d\n", med[0].name, med[0].quantity, city, companyID);*/
			/*---------TESTING----------*/

			while (waitpid(pids[0], &status, WNOHANG) == 0)
			{
				printf("waiting for map to end...\n");
				sleep(1);
			}
			if( WIFEXITED(status) )
			{
				printf("termino el mapa\n");
				endServer(server);
			}
	}

	/*wait(&rv);
	printf("termina el mapa\n");*/

/*	startSimulation();*/
/*	freeResources();*/

	return 0;
}
