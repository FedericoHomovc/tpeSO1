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
#include "structs.h"
#include "backEnd.h"

int
main(int argc, char * argv[])
{
	mapData * mapFile;
	int notValid, i;
	map * mapSt;
	pid_t pid;
	pid_t pid2;
	int status, rv;
	int k = 0;
	int * pids;

	if (argc<=2)
	{
		printf("Invalid arguments\n");
		return 1;
	}

	if(allocMapData(&mapFile))
	{
		return 1;
	}
	if(!openFile(mapFile, argv[1]))
	{
		allocMapSt(&mapSt, argc);
		/*creation of cities*/
		notValid = createCities(mapFile, mapSt);
		if(notValid)
		{
			printf("File Error\n");
			return notValid;
		}
	}
	else
	{
		printf("Impossible to open file\n");
		return 1;
	}
	/*creation of companies*/
	for( i = 2; i < argc; i++)
	{
		if( !openFile(mapFile, argv[i]) )
		{
			if(createCompany(mapFile, &mapSt->companies[i-2]))
			{
				printf("File Error\n");
				return 1;
			}
			mapSt->companies[i-2]->ID = i-2;
		}
		else
		{
			printf("Impossible to open file\n");
			return 1;
		}
	}
	mapSt->companiesCount = argc - 2;
	
	switch( pid = fork() ){
		case -1:
			perror("creating map");
			exit(1);
		case 0:
			printf("empieza el mapa\n");
			execl("map", "map", (char *) 0);
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
					pids = malloc(sizeof(int) * mapSt->companiesCount);
					while(k < mapSt->companiesCount)
					{
						switch( pids[k] = fork() ){
							case -1:
								perror("creating company");
							case 0:
								printf("empieza la company %d\n", k);
								execl("company", "company", (char *) 0);
								k++;
							default:
								;
								
						}
						k++;
					}
			}
			printf("teta peluda\n");
			waitpid(pid, &status, WNOHANG);
			if( WIFEXITED(status) )
			{
				printf("termino el mapa\n");
				kill(pid2, SIGTERM);
				kill(pids[0], SIGTERM);
			}
	}

	/*wait(&rv);
	printf("termina el mapa\n");*/

/*	startSimulation();*/
/*	freeResources();*/

	return 0;
}
