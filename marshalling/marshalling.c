/***
 ***
 ***		marshalling.c
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
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <error.h>

/***		Project Includes		***/
#include "../include/api.h"
#include "../include/structs.h"


char * wrappMedicine(medicine ** med, int companyID, int medCount);
int unwrappMedicine(medicine *** meds, char * array);
char * wrappMap(int size, int ** map);
int unwrappMap(char * array, int * size, int *** mapRcv);
void itoa(int n, char *string);

int
sendPlane(plane * p, comuADT client)
{
	message msg;
	/*char * aux;*/

	if( (msg.message = wrappMedicine(p->medicines, p->companyID, p->medCount)) == NULL )
		return -1;

	msg.size = strlen(msg.message);

	/*---------TESTING----------*/
	printf("plane sent: %s\n", (char*)msg.message );
	printf("plane sent size: %ld\n",msg.size);
	/*---------TESTING----------*/


	return sendMsg(client, &msg, 0);
}


int
rcvPlane(plane ** p, comuADT client)
{
	message msg;
	int ret;

	if( (ret = rcvMsg(client, &msg, IPC_NOWAIT)) == -1 )
		return -1;

	/*---------TESTING----------*/
	printf("pack med rcv: %s\n", (char *)msg.message);
	printf("rcv size: %ld\n",msg.size);
	/*---------TESTING----------*/

	/*unwrappMedicine(city, companyID, planeID, med, (char *)msg.message));*/

	return ret;

}


char *
wrappMedicine(medicine ** med, int companyID, int medCount)
/*format: companyID;medCount;med1,cant;med2,cant;...0*/
{
	int i;
	char * number = NULL;
	char * aux = NULL;

	aux = calloc(30, sizeof(char));	/*set to serialize companyID and medCount. initial size may be increased*/
	number = calloc(10, sizeof(char));
	
	itoa(companyID, number);
	strcat(aux, number);
	strcat(aux, ";");
	itoa(medCount, number);
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
	
	return aux;
}


int
sendMap(int size, int ** map, city ** cities, comuADT client)
{
	message msg;
	char * aux;
	int i;

	if( (msg.message = wrappMap(size, map)) == NULL )
		return -1;

	for(i = 0; i<size; i++)
	{
		if( (aux = wrappMedicine(cities[i]->medicines, cities[i]->ID, cities[i]->medCount)) == NULL)
			return -1;
		if( (msg.message = realloc(msg.message, strlen((char*)msg.message) + strlen(aux) + 1)) == NULL)
			return -1;
		strcat(msg.message, aux);
		free(aux);
	}

	msg.size = strlen((char *) msg.message);
	
	/*---------TESTING----------*/
	/*printf("map sent: %s\n", (char*)msg.message );
	printf("send size: %ld\n",msg.size);*/
	/*---------TESTING----------*/

	return sendMsg(client, &msg, 0);
	
}

int
unwrappMedicine(medicine *** meds, char * array)
{
	int i = 0, k, pos = 0, size;
	char aux[20];				/*max medicine name length = 20. may be increased if necessary*/
	medicine ** m;
	
	while(array[i] != ';')
		i++;
	i++;
	while(array[i] != ';')
		aux[pos++] = array[i++];
	aux[pos] = 0;
	size = atoi(aux);
	
	m = malloc(sizeof(medicine *) * (size + 1));

	i++;
	for(pos = 0, k = 0; pos < size; pos++, k = 0)
	{
		m[pos] = malloc(sizeof(medicine));
		while(array[i] != ',')
			aux[k++] = array[i++];
		aux[k] = 0;
		m[pos]->name = malloc(strlen(aux) + 1);
		strcpy(m[pos]->name, aux);
		k = 0; i++;
		while(array[i] != ';')
			aux[k++] = array[i++];
		aux[k] = 0;
		m[pos]->quantity = atoi(aux);
		i++;
		/*printf("name: %s, quantity: %d\n", m[pos]->name, m[pos]->quantity);*/
	}
	m[pos] = NULL;

	*meds = m;
	
	return i;
}


int
rcvMap(int *** map, medicine **** meds, comuADT client, int * size)
{
	message msg;
	int ret, i, k = 0;
	medicine *** m = NULL;

	/* TESTING */
	msg.size = 500;		/*size 500 to be set as global variable*/
	/* TESTING */

	if( (msg.message = malloc(500 * sizeof(int))) == NULL )  /*size 500 to be set as global variable*/
		return -1;
	if( (ret = rcvMsg(client, &msg, 0)) == -1 )
		return -1;
	/*---------TESTING----------*/
	/*printf("map rcv: %s\n", (char *)msg.message);
	printf("rcv size: %ld\n", msg.size);*/
	/*---------TESTING----------*/

	if( (i = unwrappMap((char *) msg.message, size, map)) == -1)
		return -1;
	
	m = malloc(sizeof(medicine *) * (* size));

	for(k = 0; k<*size; k++)
		i += unwrappMedicine(&m[k], (char *)msg.message + i);

	*meds = m;
	free(msg.message);

	return ret;
}


char *
wrappMap(int size, int ** map)
/*format: size;11;12;...;1N;21;22;...;2N;...;N1;N2;...;NN;*/
{
	int i, j;
	char * aux = NULL;
	char * number = NULL;
	
	if( (aux = calloc( size * size * 3, sizeof(char) )) == NULL ) /*if avg distances >= 100 parameter '3' must be increased*/
		return NULL;
	if ( (number = malloc(10)) == NULL)
		return NULL;
	
	itoa(size, number);
	strcat(aux, number);
	strcat(aux, ";");
	
	for(i = 0; i < size; i++)
		for(j = 0; j < size; j++)
		{
			itoa(map[i][j], number);
			strcat(aux, number);
			strcat(aux, ";");
		}
	free(number);

	i = strlen(aux);
	aux = realloc(aux, i + 1);
	aux[i] = 0;
	
	return aux;
}

int
unwrappMap(char * array, int * retSize, int *** mapRcv)
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
		return -1;
	for(k = 0; k < size; k++ )
		if ( (map[k] = malloc(size * sizeof(int))) == NULL)
			return -1;
	
	i++;
	while(pos < size * size)
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
	*mapRcv = map;

	return i;
}
