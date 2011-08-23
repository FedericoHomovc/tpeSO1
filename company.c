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

/***		Project Includes		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/api.h"
#include "./include/marshalling.h"

int canUnload(plane ** p, int ** map, medicine *** med);
map * mapSt;

int
companyFunc(processData * pdata, char * fileName , int companyID)
{
	mapData * mapFile;
	company * compa;
	comuADT client;
	medicine *** med;
	int ** map;
	int size, i;
	plane * planes;

	printf("soy la company %d\n", companyID);
	client = connectToServer(pdata->server);
	
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

	while(1)
	{
		printf("Company %d sleeping\n", compa->ID);
		raise(SIGSTOP);
		printf("Company %d awake\n", compa->ID);

		printf("Recieved: %d\n", rcvMap(&map, &med, client, &size));
		printf("planes count:%d\n", compa->planesCount);

		for(i = 0; i < compa->planesCount; i++)
		{
			printf("teta\n");
			if( canUnload(&(compa->companyPlanes[i]), map, med) )
				printf("plane:%d company:%d startCity:%d. Unloading.\n", i, companyID, compa->companyPlanes[i]->destinationID);
		}
		
	}

	return 1;
}



int
canUnload(plane ** p, int ** map, medicine *** med)
{
	int i, j;
	
	if((*p)->distance > 0)
	{
		(*p)->distance--;
		return 0;
	}

	for(i = 0; med[(*p)->destinationID][i] != NULL; i++)
	{
		if( med[(*p)->destinationID][i]->quantity != 0 )
			for(j = 0; j < (*p)->medCount; j++)
			{
				printf("Plane med: %s City med: %s\n", (*p)->medicines[j]->name, med[(*p)->destinationID][i]->name);
				if(! strcmp( (*p)->medicines[j]->name, med[(*p)->destinationID][i]->name ) )
					return 1;
			}
	}
	return 0;
}







