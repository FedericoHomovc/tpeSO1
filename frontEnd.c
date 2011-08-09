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

/***		Project Includes		***/
#include "structs.h"
#include "backEnd.h"

int
main(int argc, char * argv[])
{
	mapData * mapFile;
	int notValid, i;
	map * mapSt;

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
		/*creaction of cities*/
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
	/*creation of the companies*/
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

/*	startSimulation();*/
/*	freeResources();*/

	return 0;

}
