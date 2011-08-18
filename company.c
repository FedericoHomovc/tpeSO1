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

/***		Project Includes		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/api.h"

int
companyFunc(processData * pdata, char * fileName , int companyID)
{
	mapData * mapFile;
	company * compa;
	comuADT client;

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

	/*---------TESTING----------*/
	printf("%d\n", sendPackage(compa->companyPlanes[0]->destinationID, compa->companyPlanes[0]->medicines, client, compa->ID, compa->companyPlanes[0]->planeID, compa->companyPlanes[0]->medCount));
	/*---------TESTING----------*/

	sleep(10000);
	exit(0);
}
