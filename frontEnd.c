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
	
	
	if( (pid = fork()) < 0)
		perror("teta");
	if(pid == 0)
	{
		printf("empieza el mapa\n");
		execl("map", "map", (char *) 0);
		/*_exit(0);*/
	}
	else
	{	
		if( (pid2 = fork()) < 0)
			perror("teta");
		if(pid2 == 0)
		{
			printf("empieza IO\n");
			execl("io", "io", (char *) 0);
			/*_exit(0);*/
		}
		else
		{
			wait(&rv);
			printf("termina IO\n");
		}	
/*		wait(&rv);
		printf("termina el mapa\n");*/
	}

	pids = malloc(sizeof(int) * mapSt->companiesCount);
	
	while(k < mapSt->companiesCount)
	{
		if( (pids[k] = fork()) < 0)
			perror("teta");
		if(pids[k] == 0)
		{
			printf("empieza la company %d\n", k);
			execl("company", "company", (char *) 0);
		}
		else
		{
			wait(&rv);
			printf("termina la company %d\n", k);
		}
		k++;
	}
	
	wait(&rv);
	printf("termina el mapa\n");

	


		/*if( waitpid(pid, &status, WNOHANG) == 0)	
		{
			printf("murio el mapa, mueran putos\n");
			exit(0);
		}*/

/*	startSimulation();*/
/*	freeResources();*/

	return 0;
}
