/***
***
***		map.c
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
#include <string.h>

/***		Project Includes		***/
#include "./include/api.h"
#include "./include/varray.h"
#include "./include/structs.h"
#include "./include/backEnd.h"

int
mapFunc(processData * pdata, char * fileName , int argc)
{
	mapData * mapFile;
	int notValid;
	map * mapSt;
	comuADT client;
	/*message msg;*/

	printf("soy el mapa\n");
	strcpy( pdata->name, "map" );
	client = connectToServer(pdata->server);

	if(allocMapData(&mapFile))
	{
		return 1;
	}
	if(!openFile(mapFile, fileName))
	{
		allocMapSt(&mapSt, argc);
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

	sleep(2);
	
	return 0;
}
