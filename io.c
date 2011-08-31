/***
***
***		io.c
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
#include <sys/msg.h>

/***		Project Includes		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/api.h"
#include "./include/marshalling.h"

static void sigintServHandler(int signo);
void freeIOResources(void);

comuADT mapClient;
map * mapSt;

int
ioFunc(processData * pdata)
{
	comuADT client;
	medicine *** med;
	int size, i, j;

	struct sigaction signalAction;
	signalAction.sa_flags = 0;
	signalAction.sa_handler = sigintServHandler;
	sigfillset(&(signalAction.sa_mask));

	sigaction(SIGINT, &signalAction, NULL);
	
	client = connectToServer(pdata->server);
	sendChecksign(mapClient);
	while(TRUE)
	{
		rcvMap(&med, client, &size);
		/*printf("\nMap cities distances:\n");
		for(i = 0; i < size; i++)
		{
			for(j = 0; j < size; j++)
				printf("%d\t", map[i][j]);
			printf("\n");
		}*/
		for(i = 0; i < size; i++)
		{
			printf("\nCity %d requests:\n", i);
			printf("Medicine\t\t\tQuantity\n");
			for(j = 0; med[i][j] != NULL; j++)
			{
				printf("%s\t\t\t%d", med[i][j]->name, med[i][j]->quantity);
				printf("\n");
			}
		}
		printf("\n");
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
	}

	return 1;
}

static void sigintServHandler(int signo) {
	freeIOResources();
	exit(0);
}


void
freeIOResources(void)	/*si es la misma direccion de memoria que map.c para la variable mapSt no hace falta liberar nada*/
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
