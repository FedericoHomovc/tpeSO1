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
#include <error.h>


/***		Project Includes		***/
#include "./include/api.h"
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/marshalling.h"
#include "./include/semaphore.h"

/***		Functions		***/
int companyFunc(processData * pdata, char * fileName, int companyID);
int ioFunc(processData * pdata, int companyCount);
int unloadPlane(plane ** p, map ** mapSt);
int freeResources(void);
int needMedicines(map * mapSt);



map * mapSt;
comuADT mapClient;
int semmapid;

int 
main(int argc, char * argv[]) {

	processData * pdata;
	int k, companyID, count, i, j, c, unloaded;
	pid_t * pids;
	servADT server;
	comuADT * clients;
	FILE * file = NULL;
	plane ** p;


	if (argc <= 2) {
		printf("Invalid arguments\n");
		return 1;
	}

	if (!openFile(&file, argv[1])) {
		allocMapSt(&mapSt, argc);
		if (createCities(file, mapSt)) {
			printf("Map File Error\n");
			return 1;
		}
		/*fclose(file);*/
	} else {
		printf("Impossible to open file\n");
		return 1;
	}

	server = startServer();
	pdata = malloc(sizeof(processData));
	pdata->server = server;
	pdata->pid = getpid();

	if( (pids = malloc(sizeof(pid_t) * (argc))) == NULL )
		return 1;
	if( (clients = malloc(sizeof(comuADT *) * (argc))) == NULL)
		return 1;

	pids[0] = getpid();
	if( (mapClient = clients[0] = connectToServer(server)) == NULL) /*map client*/
	{
		return 1;
	}

	switch (pids[1] = fork()) {
	case -1:
		perror("Error creating IO");
		exit(1);
	case 0:
		if(ioFunc(pdata, argc - 2))
			printf("Error during IO execution\n");
		_exit(0);
	default:
		k = 2;
		while (k < argc) {
			switch (pids[k] = fork()) {
			case -1:
				perror("Error creating company");
			case 0:
				if (companyFunc(pdata, argv[k], k - 2))
					printf("Error during company %d execution\n", k - 2);
				_exit(0);
			}
			k++;
		}
	}

	k = 0;
	while(k < argc - 1) /*wait for all processes to connect to server*/
	{
		if( rcvChecksign(clients[0]) )
		{
			printf("Error connecting processes.\n");
			return 1;
		}
		k++;
	}
		
	if( (clients[1] = getClient(server, pids[1])) == NULL )		/*ioClient*/
	{
		printf("Error connecting processes. PID %d not connected.\n", pids[1]);
		return 1;
	}
	for (k = 2; k < argc; k++)
		if( (clients[k] = getClient(server, pids[k])) == NULL)
		{
			printf("Error connecting processes. PID %d not connected.\n", pids[k]);
			return 1;
		}

	while(needMedicines(mapSt))
	{
		sendMap(mapSt->citiesCount, mapSt->cities, clients[1]);
		if( rcvChecksign(mapClient) )
		{
			printf("Error during IPC @ map.c\n");
			return 1;
		}

		/*do{
			c = getchar();
		}while(c != '\n');*/

		for(k = 2; k < argc; k++)
			sendMap(mapSt->citiesCount, mapSt->cities, clients[k]);
		
		k = 0;
		while(k < argc - 2)
		{
			rcvPlanes(&companyID, &count, &p, clients[0]);
			unloaded = 0;
			if(count > 0)
			{
				for(i = 0; i < count; i++)
					unloaded += unloadPlane(&p[i], &mapSt);

				sendPlanes(companyID, count, p, clients[companyID+2]);
			}
			if(unloaded)
				sendPlanes(companyID, count, p, clients[1]);
			else
				sendPlanes(companyID, 0, p, clients[1]);
			for(i = 0; i < count; i++)
			{
				for(j = 0; j < p[i]->medCount; j++)
				{
					free(p[i]->medicines[j]->name);
					free(p[i]->medicines[j]);
				}
				free(p[i]);
			}
		k++;
		}

		for(k = 2; k < argc; k++)
			sendMap(mapSt->citiesCount, mapSt->cities, clients[k]);
		
		if( rcvChecksign(clients[0]) )
		{
			printf("Error during IPC @ map.c\n");
			return 1;
		}
	}

	sendMap(mapSt->citiesCount, mapSt->cities, clients[1]);
	rcvChecksign(clients[0]);

	disconnectFromServer(clients[0], server);
	for(k = 1; k < argc; k++)
		kill(pids[k], SIGINT);	/*SIGTERM o SIGINT?*/
	endServer(server);

	freeResources();
	free(pids);
	free(clients);
	free(pdata);
	printf("Simulation ended\n");
	return 0;
}


int
unloadPlane(plane ** p, map ** mapSt)
{
	int i, j, ret = 0;
	city * cty;

	cty = (*mapSt)->cities[(*p)->destinationID];

	for(i = 0; i < (*mapSt)->cities[(*p)->destinationID]->medCount; i++)
	{
		for(j = 0; (*p)->medicines[j] != NULL; j++)
		{
			if( cty->medicines[i]->quantity > 0 && (*p)->medicines[j]->quantity > 0 && !strcmp((*p)->medicines[j]->name, cty->medicines[i]->name))
			{
				if( (*p)->medicines[j]->quantity >= cty->medicines[i]->quantity)
				{
					(*p)->medicines[j]->quantity -= cty->medicines[i]->quantity;
					cty->medicines[i]->quantity = 0;
				}
				else{
					cty->medicines[i]->quantity -= (*p)->medicines[j]->quantity;
					(*p)->medicines[j]->quantity = 0;
				}
				ret = 1;
			}
		}
	}		
	
	return ret;
}


int
freeResources(void)
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

	return 0;
}


int
needMedicines(map * mapSt)
{
	int i, j;
	for(i = 0; i< mapSt->citiesCount; i++)
		for(j = 0; j<mapSt->cities[i]->medCount; j++)
			if(mapSt->cities[i]->medicines[j]->quantity > 0)
				return 1;

	return 0;
}

