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
#include "./include/marshalling.h"

int companyFunc(processData * pdata, char * fileName, int companyID);
int ioFunc(processData * pdata);
int unloadPlane(plane ** p, map ** mapSt);
int freeResources(void);
int needMedicines(map * mapSt);

map * mapSt;
comuADT mapClient;

int 
main(int argc, char * argv[]) {

	processData * pdata;
	int k, notValid, companyID, count, i,  turn = 0;
	pid_t * pids;
	servADT server;
	comuADT * clients;
	mapData * mapFile;
	plane ** p;

	server = startServer();
	pdata = malloc(sizeof(processData));
	pdata->server = server;
	pdata->pid = getpid();
	pdata->name = malloc(sizeof(char *) * 20); /* tamaño del string */

	pids = malloc(sizeof(pid_t) * (argc)); /*poner en el back*/
	clients = malloc(sizeof(comuADT *) * (argc)); /*poner en el back*/
	pids[0] = getpid();
	mapClient = clients[0] = connectToServer(server); /*map client*/

	if (argc <= 2) {
		printf("Invalid arguments\n");
		return 1;
	}

	strcpy(pdata->name, "map");

	if (allocMapData(&mapFile)) {
		return 1;
	}
	if (!openFile(mapFile, argv[1])) {
		allocMapSt(&mapSt, argc);
		notValid = createCities(mapFile, mapSt);
		if (notValid) {
			printf("File Error\n");
			return notValid;
		}
	} else {
		printf("Impossible to open file\n");
		return 1;
	}

	switch (pids[1] = fork()) {
	case -1:
		perror("Error creating IO");
		exit(1);
	case 0:
		ioFunc(pdata);
		_exit(0);
	default:
		k = 2;
		while (k < argc) {
			switch (pids[k] = fork()) {
			case -1:
				perror("Error creating company");
			case 0:
				if (companyFunc(pdata, argv[k], k - 2))
					printf("Error opening company %d file\n", k);
				_exit(0);
			}
			k++;
		}
	}
	
	sleep(1);
	clients[1] = getClient(server, pids[1]);/*ioClient*/
	for (k = 2; k < argc; k++) {
		clients[k] = getClient(server, pids[k]);
	}

	while(needMedicines(mapSt))
	{
		printf("turn: %d\n", turn++);
		sendMap(mapSt->citiesCount, mapSt->graph, mapSt->cities, clients[1]);
		rcvChecksign(clients[0]);
		sleep(1); /*para que se pueda ver el mapa*/
			
		for(k = 2; k < argc; k++)
		{
			sendMap(mapSt->citiesCount, mapSt->graph, mapSt->cities, clients[k]);
		}

		for(k = 2; k < argc; k++)
		{
			rcvPlanes(&companyID, &count, &p, clients[0]);
			if(count > 0)
			{
				for(i = 0; i < count; i++)
					unloadPlane(&p[i], &mapSt);

				sendPlanes(companyID, count, p, clients[companyID+2]);
				rcvChecksign(clients[0]);
				sendMap(mapSt->citiesCount, mapSt->graph, mapSt->cities, clients[companyID+2]);
			}
		}
	}

	sendMap(mapSt->citiesCount, mapSt->graph, mapSt->cities, clients[1]);
	rcvChecksign(clients[0]);


	disconnectFromServer(clients[0], server);
	for(k = 1; k < argc; k++)
		kill(pids[k], SIGTERM);
	endServer(server);

	freeResources();
	printf("Simulation ended\n");
	return 0;
}


int
unloadPlane(plane ** p, map ** mapSt)
{
	int i, j, ret = 1;

	for(i = 0; i < (*mapSt)->cities[(*p)->destinationID]->medCount; i++)
	{
		for(j = 0; (*p)->medicines[j] != NULL; j++)
			if((*mapSt)->cities[(*p)->destinationID]->medicines[i]->quantity > 0 && !strcmp((*p)->medicines[j]->name, (*mapSt)->cities[(*p)->destinationID]->medicines[i]->name))
			{
				if( (*p)->medicines[j]->quantity >= (*mapSt)->cities[(*p)->destinationID]->medicines[i]->quantity)
				{
					(*p)->medicines[j]->quantity -= (*mapSt)->cities[(*p)->destinationID]->medicines[i]->quantity;
					(*mapSt)->cities[(*p)->destinationID]->medicines[i]->quantity = 0;
				}
				else{
					(*mapSt)->cities[(*p)->destinationID]->medicines[i]->quantity -= (*p)->medicines[j]->quantity;
					(*p)->medicines[j]->quantity = 0;
				}
				ret = 0;
			}
	}		
	
	return ret;
}

int
freeResources(void)
{
	int i;
	for(i = 0; i < mapSt->citiesCount; i++)
		free(mapSt->graph[i]);	
	free(mapSt->graph);

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




