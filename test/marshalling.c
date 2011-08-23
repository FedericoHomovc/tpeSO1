/***
 ***
 ***		marshalling.c
 ***				Jose Ignacio Galindo
 ***				Federico Homovc
 ***				Nicolas Loreti
 ***			 	     ITBA 2011
 ***
 ***/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>


#include "../include/api.h"
#include "../include/structs.h"


char * wrappMedicine(int city, medicine ** med, int companyID, int planeID, int medCount);
int unwrappMedicine(int city, medicine ** med, int companyID, int planeID, char * array);
char * wrappMap(int size, int ** map);
int ** unwrappMap(char * array, int * size);
void itoa(int n, char *string);


int
sendPackage(int city, medicine ** med, comuADT client, int companyID, int planeID, int medCount)
{
	message msg;

	if( (msg.message = wrappMedicine(city, med, companyID, planeID, medCount)) == NULL )
		return -1;

	msg.size = strlen(msg.message);

	/*---------TESTING----------*/
	printf("pack med sent: %s\n", (char*)msg.message );
	printf("send size: %ld\n",msg.size);
	/*---------TESTING----------*/


	return sendMsg(client, &msg, 0);
}


int
rcvPackage(int * city, medicine ** med, comuADT client, int * companyID, int * planeID )
{
	message msg;
	int ret;

	if( (ret = rcvMsg(client, &msg, 0)) == -1 )
		return -1;

	/*---------TESTING----------*/
	printf("pack med rcv: %s\n", (char *)msg.message);
	printf("rcv size: %ld\n",msg.size);
	/*---------TESTING----------*/

	/*unwrappMedicine(city, companyID, planeID, med, (char *)msg.message));*/

	return ret;

}


char *
wrappMedicine(int city, medicine ** med, int companyID, int planeID, int medCount)
/*format: city;companyID;planeID;med1,cant;med2,cant;...0*/
{
	int i;
	char * number = NULL;
	char * aux = NULL;

	aux = malloc(30);	/*set to serialize city, companyID and planeID. initial size may be increased*/
	number = malloc(10);
	
	itoa(city, number);
	strcat(aux, number);
	strcat(aux, ";");
	itoa(companyID, number);
	strcat(aux, number);
	strcat(aux, ";");
	itoa(planeID, number);
	strcat(aux, number);
	strcat(aux, ";");
	
	for( i = 0; i < medCount; i++)
	{
		if( ( aux = realloc(aux, strlen(aux) + strlen(med[i]->name) + sizeof(int) + 2)) == NULL )
			return NULL;
		strcat(aux, med[i]->name);
		strcat(aux, ",");
		itoa(med[i]->quantity, number);
		if( number == NULL )
			return NULL;
		strcat(aux, number);
		strcat(aux, ";");
	}
	free(number);
	aux = realloc( aux, strlen(aux));

	return aux;
}

int
unwrappMedicine(int city, medicine ** med, int companyID, int planeID, char * array)
{
	*med = malloc(sizeof(medicine));
	med[0]->name = array;
	med[0]->quantity = 123;
	return 0;
}

int
sendMap(int size, int ** map, comuADT client)
{
	message msg;

	if( (msg.message = wrappMap(size, map)) == NULL )
		return -1;

	msg.size = strlen((char *) msg.message);
	
	/*---------TESTING----------*/
	printf("map sent: %s\n", (char*)msg.message );
	printf("send size: %ld\n",msg.size);
	/*---------TESTING----------*/

	return sendMsg(client, &msg, 0);
	
}

int
rcvMap(int *** map, comuADT client, int * size)
{
	message msg;
	int ret;

	/* TESTING */
	msg.size = 200;		/*size 200 to be set as global variable*/
	/* TESTING */

	if( (msg.message = malloc(200 * sizeof(int))) == NULL )  /*size 200 to be set as global variable*/
		return -1;
	if( (ret = rcvMsg(client, &msg, 0)) == -1 )
		return -1;
	/*---------TESTING----------*/
	/*printf("map rcv: %s\n", (char *)msg.message);
	printf("rcv size: %ld\n", msg.size);*/
	/*---------TESTING----------*/

	if( (*map = unwrappMap((char *) msg.message, size)) == NULL)
		return -1;

	free(msg.message);

	return ret;
}


char *
wrappMap(int size, int ** map)
/*format: size;11;12;...;1N;21;22;...;2N;...;N1;N2;...;NN;*/
{
	int i = 0, j;
	char * aux = NULL;
	char * number = NULL;
	
	if( (aux = malloc( size * size * 3 )) == NULL ) /*if avg distances >= 100 parameter '3' must be increased*/
		return NULL;
	if ( (number = malloc(10)) == NULL)
		return NULL;
	
	itoa(size, number);
	strcat(aux, number);
	strcat(aux, ";");
	
	for(i = 0; i < size; i++)
		for(j = 0; j < size; j ++)
		{
			itoa(map[i][j], number);
			strcat(aux, number);
			strcat(aux, ";");
		}

	free(number);
	return aux;
}

int **
unwrappMap(char * array, int * retSize)
{
	char aux[10];
	int ** map = NULL;
	int i = 0, k, size, number, pos = 0;
	
	/*printf("unwrapping %s\n", array);*/
	while(array[i] != ';' )
	{
		aux[i] = array[i];
		i++;
	}
	size = atoi(aux);
	
	if ( (map = malloc(size * sizeof(int) )) == NULL)
		return NULL;
	for(k = 0; k < size; k++ )
		if ( (map[k] = malloc(size * sizeof(int))) == NULL)
			return NULL;
	
	i++;
	while(array[i])
	{
		k = 0;
		while(array[i] != ';')
		{
			aux[k++] = array[i];
			i++;
		}
		aux[k] = 0;
		number = atoi(aux);
		map[pos/size][pos%size] = number;
		pos++; i++;
	}

	*retSize = size;

	return map;
}
