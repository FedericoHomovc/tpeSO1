/***
***
***		company.c
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
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <error.h>

/***		Project Includes		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/api.h"
#include "./include/marshalling.h"

int canUnload(plane ** p, int ** map, medicine *** med);
int checkPlaneCargo(company ** company, plane ** p, int count);
int setPlaneDestination(company ** compa, int ** map, int size);
map * mapSt;
comuADT mapClient;

int
companyFunc(processData * pdata, char * fileName , int companyID)
{
	mapData * mapFile;
	company * compa;
	comuADT client;
	medicine *** med;
	int ** map;
	int size, i, j, unloading;
	plane ** planes, ** p;

	client = connectToServer(pdata->server);
	sendChecksign(mapClient);
	
	if(allocMapData(&mapFile))
	{
		return 1;
	}
	if( !openFile(mapFile, fileName) )
	{
		if(createCompany(mapFile, &compa))
		{
			printf("File Error\n");
			return 1;
		}
	}
	else
	{
		printf("Impossible to open file\n");
		return 1;
	}
	compa->ID = companyID;
	for(i = 0; i < compa->planesCount; i++)
		if ( (compa->companyPlanes[i]->destinationID = getCityID(compa->companyPlanes[i]->startCity, mapSt)) == -1)
		{
			printf("Invalid start city for plane %d in company %d.\n", i, compa->ID);
			return 1;
		}

	planes = malloc(sizeof(plane *) * compa->planesCount);

	while(1)
	{	
		rcvMap(&map, &med, client, &size);
		unloading = 0;
		for(i = 0; i < compa->planesCount; i++)
			if( canUnload(&(compa->companyPlanes[i]), map, med) )
				planes[unloading++] = compa->companyPlanes[i];

		for(i = 0; i < size; i++)
			free(map[i]);
		free(map);
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

		if(unloading == 0)
			sendPlanes(compa->ID, unloading, NULL, mapClient);

		else
		{
			sendPlanes(compa->ID, unloading, planes, mapClient);
			
			rcvPlanes(NULL, &unloading, &p, client);
			checkPlaneCargo(&compa, p, unloading);
			for(i = 0; i < unloading; i++)
			{
				for(j = 0; j < p[i]->medCount; j++)
				{
					free(p[i]->medicines[j]->name);
					free(p[i]->medicines[j]);
				}
				free(p[i]);
			}
		}
		rcvMap(&map, &med, client, &size);
		setPlaneDestination(&compa, map, size);
		for(i = 0; i < size; i++)
			free(map[i]);
		free(map);
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



int
canUnload(plane ** p, int ** map, medicine *** med)
{
	int i, j;
	
	(*p)->distance--;
	if((*p)->distance >= 0)
	{
		return 0;
	}

	for(i = 0; med[(*p)->destinationID][i] != NULL; i++)
	{
		if( med[(*p)->destinationID][i]->quantity != 0 )
			for(j = 0; j < (*p)->medCount; j++)
				if(! strcmp( (*p)->medicines[j]->name, med[(*p)->destinationID][i]->name ) )
					return 1;
	}
	return 0;
}

int
checkPlaneCargo(company ** cmp, plane ** p, int count)
{
	int i, j;
	for(i = 0; i < count; i++)
		for(j=0; j< (*cmp)->companyPlanes[p[i]->planeID]->medCount; j++)
			(*cmp)->companyPlanes[p[i]->planeID]->medicines[j]->quantity = p[i]->medicines[j]->quantity;

	return 0;
}

int
setPlaneDestination(company ** compa, int ** map, int size)
{
	int i, next = 1, cityID;
	for(i = 0; i < (*compa)->planesCount; i++)
		if( (*compa)->companyPlanes[i]->distance == -1)
		{
			cityID = (*compa)->companyPlanes[i]->destinationID;
			while( map[cityID][ (cityID + next) % size] == 0)
				next++;
			(*compa)->companyPlanes[i]->originID = (*compa)->companyPlanes[i]->destinationID;
			(*compa)->companyPlanes[i]->distance = map[cityID][ (cityID + next) % size];
			(*compa)->companyPlanes[i]->destinationID = (cityID + next) % size;
		}
	return 0;
}




