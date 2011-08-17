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
#include "structs.h"
#include "backEnd.h"

int
main(int argc, char * argv[])
{
	mapData * mapFile;
	company * compa;

	printf("soy una company\n");
	
	if(allocMapData(&mapFile))
	{
		return 1;
	}
	if( !openFile(mapFile, argv[1]) )
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

	sleep(10000);
	exit(0);
}
