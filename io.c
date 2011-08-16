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


/***		Project Includes		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/api.h"
#include "./include/varray.h"
#include "./include/fifo.h"
#include "./include/io.h"



int
main()
{

	clients[1] = connectToServer(server);
	printf("soy IO\n");
	sleep(10000);
	return 0;
}
