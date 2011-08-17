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

/***		Project Includes		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/api.h"
#include "./include/varray.h"

int
main(int argc, char * argv[])
{
	mapData * mapFile;
	int notValid;
	map * mapSt;
	comuADT front;
	message msg;

	/*front = getClient(server, getppid());*/

	printf("soy el mapa\n");

	if(allocMapData(&mapFile))
	{
		return 1;
	}
	if(!openFile(mapFile, argv[1]))
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

	/*rcvMsg(front, &msg, 0);*/
	
	sleep(2);

	return 0;
}
