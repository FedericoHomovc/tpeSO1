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
#include "structs.h"
#include "backEnd.h"
#include "./include/api.h"
#include "./include/varray.h"

servADT server;
comuADT * clients;

int
main()
{
	message * msg = NULL;
	printf("soy el mapa\n");
	clients[1] = connectToServer(server); /*map client*/
	sleep(1);
	rcvMsg(clients[0], msg, 0);

	printf("%s\n", (char *)msg->message);

	return 0;
}
