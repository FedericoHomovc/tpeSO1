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


char * wrappMedicine(medicine ** med, int ID, int medCount);
int unwrappMedicine(medicine *** meds, char * array, int * ID);
char * wrappMap(int size, int ** map);
int unwrappMap(char * array, int * size, int *** mapRcv);
void itoa(int n, char *string);


int
sendChecksign(comuADT client)
{
	message msg;
	msg.message = "OK";
	msg.size = 500;

	return sendMsg(client, &msg, 0);
}

int
rcvChecksign(comuADT client)
{
	message msg;

	return rcvMsg(client, &msg, 0);
}


int
sendPlanes(int companyID, int count, plane ** p, comuADT client)
/*Format: companyID;count;plane1ID;destination1ID;medCount1;med1,c1;med2,c2;...;plane2ID;destination2ID;medCount2;med1,c1;...;*/
{
	message msg;
	char * aux = NULL;
	char * num = NULL;
	int i;

	
	num = calloc(10, sizeof(char));
	msg.message = calloc(15, sizeof(char));

	itoa(companyID, msg.message);
	strcat(msg.message, ";");
	itoa(count, num);
	strcat(msg.message, num);
	strcat(msg.message, ";");

	if(count == 0)
		return sendMsg(client, &msg, 0); 

	for(i = 0; i< count; i++)
	{
		if( (aux = wrappMedicine(p[i]->medicines, p[i]->destinationID, p[i]->medCount)) == NULL )
			return -1;
		msg.message = realloc(msg.message, strlen(msg.message) + strlen(aux) + 5);
		itoa(p[i]->planeID, num);
		strcat(msg.message, num);
		strcat(msg.message, ";");
		strcat(msg.message, aux);
		free(aux);
	}

	/*msg.size = strlen(msg.message);*/
	msg.size = 500;

	/*---------TESTING----------*/
	/*printf("plane sent: %s\n", (char*)msg.message );
	printf("plane sent size: %ld\n",msg.size);*/
	/*---------TESTING----------*/
	free(num);
	

	return sendMsg(client, &msg, 0);
}


int
rcvPlanes(int * companyID, int * count, plane *** p, comuADT client)
/*Format: companyID;count;destination1ID;medCount1;med1,c1;med2,c2;...;destination2ID;medCount2;med1,c1;...;*/
{
	message msg;
	int ret, i = 0, pos, j, medCount;
	char * aux = NULL;
	plane ** retPlane;

	
	if( (ret = rcvMsg(client, &msg, 0)) == -1 )
		return -1;

	/*---------TESTING----------*/
	/*printf("planes rcv: %s\n", (char *)msg.message);
	printf("rcv size: %ld\n",msg.size);*/
	/*---------TESTING----------*/

	aux = calloc(10, sizeof(char));
	pos = 0;

	while( ((char *)msg.message)[i] != ';')
		aux[pos++] = ((char *)msg.message)[i++];

	i++;
	if(companyID != NULL)
		*companyID = atoi(aux);
	
	pos = 0;
	while( ((char *)msg.message)[i] != ';')
		aux[pos++] = ((char*)msg.message)[i++];
	i++;
	aux[pos] = 0;
	*count = atoi(aux);

	if(*count != 0)
	{
		retPlane = malloc(sizeof(plane*) * *count);
		for(j = 0; j < *count; j++)
		{
			if ( (retPlane[j] = malloc(sizeof(plane))) == NULL)
				return -1;
			pos = medCount = 0;
			while( ((char *)msg.message)[i] != ';')
				aux[pos++] = ((char*)msg.message)[i++];
			i++; aux[pos] = 0;
			retPlane[j]->planeID = atoi(aux);
			i += unwrappMedicine(&retPlane[j]->medicines, (char *)msg.message + i, &retPlane[j]->destinationID);
			while(retPlane[j]->medicines[medCount] != NULL)
				medCount++;
			retPlane[j]->medCount = medCount;
		}
	}
	*p = retPlane;
	free(aux);
	/*free(msg.message);*/

	return ret;
}


char *
wrappMedicine(medicine ** med, int ID, int medCount)
/*format: ID;medCount;med1,cant;med2,cant;...0*/
{
	int i;
	char * number = NULL;
	char * aux = NULL;

	aux = calloc(10, sizeof(char));	/*set to serialize ID and medCount. initial size may be increased*/
	number = calloc(10, sizeof(char));
	
	itoa(ID, number);
	strcat(aux, number);
	strcat(aux, ";");
	itoa(medCount, number);
	strcat(aux, number);
	strcat(aux, ";");
	
	for( i = 0; i < medCount; i++)
	{
		if( ( aux = realloc(aux, strlen(aux) + strlen(med[i]->name) + 10)) == NULL )
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

	/*msg.size = strlen((char *) msg.message);*/
	msg.size = 500;
	
	/*---------TESTING----------*/
	/*printf("map sent @ marshalling.c: %s\n", (char*)msg.message );
	printf("send size: %ld\n",msg.size);*/
	/*---------TESTING----------*/

	return sendMsg(client, &msg, 0);	
}

int
unwrappMedicine(medicine *** meds, char * array, int * ID)
{
	int i = 0, k, pos = 0, size;
	char aux[20];				/*max medicine name length = 20. may be increased if necessary*/
	medicine ** m;
	
	while(array[i] != ';')
		aux[pos++] = array[i++];
	if(ID != NULL)
		*ID = atoi(aux);
	i++; pos = 0;
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

	if( (msg.message = malloc(500)) == NULL )  /*size 500 to be set as global variable*/
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
		i += unwrappMedicine(&m[k], (char *)msg.message + i, NULL);

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
