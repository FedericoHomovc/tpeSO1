/***
***
***		io.c
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
#include <sys/msg.h>


/***		Project Includes		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/api.h"
#include "./include/varray.h"
#include "./include/marshalling.h"

int
ioFunc(processData * pdata)
{
	comuADT client;
	medicine *** med;
	int ** map;
	int size, i, j;
	
	client = connectToServer(pdata->server);
	while(1)
	{
		raise(SIGSTOP);
		printf("\nMap cities distances:\n");
		rcvMap(&map, &med, client, &size);
		for(i = 0; i < size; i++)
		{
			for(j = 0; j < size; j++)
				printf("%d\t", map[i][j]);
			printf("\n");
		}
		for(i = 0; i < size; i++)
		{
			printf("\nCity %d requests:\n", i);
			printf("Medicine\tQuantity\n");
			for(j = 0; med[i][j] != NULL; j++)
			{
				printf("%s\t%d", med[i][j]->name, med[i][j]->quantity);
				printf("\n");
			}
		}
		printf("\n");
		free(map);
		free(med);

		/*kill(getppid(), SIGCONT);*/
	}

	return 1;
}
