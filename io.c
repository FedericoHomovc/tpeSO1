/***
***
*** io.c
*** Jose Ignacio Galindo
*** Federico Homovc
*** Nicolas Loreti
*** ITBA 2011
***
***/

/*** 		System includes 		***/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/msg.h>

/*** 		Project Includes 		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/api.h"
#include "./include/marshalling.h"

static void sigintServHandler(int signo);
void freeIOResources(void);

clientADT mapClient;
map * mapSt;

int
ioFunc(processData * pdata, int companyCount)
{
	clientADT client;
	medicine *** med;
	int size, i, j, k, companyID, count, planeArrived, turn = 1;
	plane ** p;

	struct sigaction signalAction;
	signalAction.sa_flags = 0;
	signalAction.sa_handler = sigintServHandler;
	sigfillset(&(signalAction.sa_mask));

	sigaction(SIGINT, &signalAction, NULL);
	if( (client = connectToServer(pdata->server)) == NULL )
	{
		printf("IO couldn't connect to server\n");
		return 1;
	}

	if( (mapClient = getClient(pdata->server, getppid())) == NULL )
	{
		printf("Map client not found\n");
		return 1;
	}

	sendChecksign(mapClient);
	while(TRUE)
	{
		planeArrived = FALSE;
		rcvMap(&med, client, &size);
		/*printf("\nMap cities distances:\n");
		for(i = 0; i < size; i++)
		{	
			for(j = 0; j < size; j++)
				printf("%d\t", mapSt->graph[i][j]);
			printf("\n");
		}*/
		printf("\nTurn: %d\n\n", turn++);
		for(i = 0; i < size; i++)
		{
			printf("***************%s requests: ***************\n", mapSt->cities[i]->name);
			printf("Medicine\t\t\tQuantity\n");
			for(j = 0; med[i][j] != NULL; j++)
				printf("%-20s\t\t%d\n", med[i][j]->name, med[i][j]->quantity);
		}
		sendChecksign(mapClient);
		for(i = 0; i < size; i++)
		{
			for(j = 0; med[i][j] != NULL; j++)
			{
				free(med[i][j]->name);
				free(med[i][j]);
			}
			free(med[i]);
		}	
		free(med);

		for(i = 0; i < companyCount; i++)
		{
			rcvPlanes(&companyID, &count, &p, client);
			if(count != 0)
			{
				planeArrived = TRUE;
				for(j = 0; j < count; j++)
printf("Company %d, plane %d arrived in %s\n", companyID, p[j]->planeID, mapSt->cities[p[j]->destinationID]->name);
			}
			for(k = 0; k < count; k++)
			{
				for(j = 0; j < p[k]->medCount; j++)
				{
					free(p[k]->medicines[j]->name);
					free(p[k]->medicines[j]);
				}
				free(p[k]);
			}
		}
		if(!planeArrived)
			printf("No plane arrived\n");
		sendChecksign(mapClient);
	}

	return 1;
}

static void sigintServHandler(int signo) {
	freeIOResources();
	exit(0);
}


void
freeIOResources(void) /*si es la misma direccion de memoria que map.c para la variable mapSt no hace falta liberar nada*/
{
	int i, j;

	for(i = 0; i < mapSt->citiesCount; i++)
		free(mapSt->graph[i]);
	free(mapSt->graph);
	for(i = 0; i < mapSt->citiesCount; i++)
	{
		for(j = 0; j < mapSt->cities[i]->medCount; j++)
		{
			free( mapSt->cities[i]->medicines[j]->name);
			free(mapSt->cities[i]->medicines[j]);
		}
		free(mapSt->cities[i]);
	}
	free(mapSt);
}
