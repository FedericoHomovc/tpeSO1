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
#include <pthread.h>

/***		Project Includes		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/api.h"

comuADT mapClient;

void * planeFunc(void * plane);

int
companyFunc(processData * pdata, char * fileName , int companyID)
{
	mapData * mapFile;
	company * compa;
	comuADT client;
	int i;
	pthread_t * threads;
	pthread_attr_t attr;
	void * status;

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
	
	threads = malloc(compa->planesCount * sizeof(pthread_t));
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);	
	
	for(i = 0; i < compa->planesCount; i++)
	{
		if (pthread_create(&threads[i], &attr, planeFunc, (void *) compa->companyPlanes[i]))
		{
			fprintf(stderr, "Can't create thread\n"); 
			exit(1);
		}
	}
	
	pthread_attr_destroy(&attr);

	for(i = 0; i < compa->planesCount; i++)
	{
		if( pthread_join(threads[i], &status) )
		{
			printf("error joining threads\n");
			exit(1);
		}
	}

	/*---------TESTING----------*/
	/*printf("%d\n", sendPackage(compa->companyPlanes[0]->destinationID, compa->companyPlanes[0]->medicines, client, compa->ID, compa->companyPlanes[0]->planeID, compa->companyPlanes[0]->medCount));*/
	/*---------TESTING----------*/

	/*disconnectFromServer(client, pdata->server);*/
	/*sleep(10000);*/
	pthread_exit(NULL);
}

void *
planeFunc(void * argPlane)
{
	plane * p;
	p = (plane *) argPlane;

	if( p->distance > 0)
	{
		p->distance--;
		/*pthread_exit(NULL);*/
	}	

	/*printf("%s\n", p->startCity);*/
	pthread_exit(NULL);
}




