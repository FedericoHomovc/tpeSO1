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
#include "../include/transport.h"
#include "../include/structs.h"

/***		Module Defines		***/
#define MED_NAME_LENGT 20		/*max medicine name length = 20. may be increased if necessary*/

/***		Functions		***/
char * wrappMedicine(medicine ** med, int ID, int medCount);
int unwrappMedicine(medicine *** meds, char * array, int * ID);
void itoa(int n, char *string);



int
sendChecksign(clientADT client)
{
	message msg;
	int ret;

	msg.message = "OK";
	msg.size = 3;

	ret = sendMessage(client, &msg, 0);
	
	return ret;
}


int
rcvChecksign(clientADT client)
{
	message msg;
	int ret;

	if( (msg.message = calloc(3, sizeof(char))) == NULL)
		return -1;
	msg.size = 3;

	if( (ret = rcvMessage(client, &msg, 0)) != -1 )
		if( strcmp((char*)msg.message, "OK") )
			return -1;

	free(msg.message);
	return ret;
}


int
sendPlanes(int companyID, int count, plane ** p, clientADT client)
/*Format: companyID;count;plane1ID;destination1ID;medCount1;med1,c1;med2,c2;...;plane2ID;destination2ID;medCount2;med1,c1;...;*/
{
	message msg;
	char * aux = NULL;
	char * num = NULL;
	int i, ret;

	
	if( (num = calloc(10, sizeof(char))) == NULL)
		return -1;
	if( (msg.message = calloc(MSG_SIZE, sizeof(char))) == NULL )
		return -1;
	msg.size = MSG_SIZE;

	itoa(companyID, msg.message);
	strcat(msg.message, ";");
	itoa(count, num);
	strcat(msg.message, num);
	strcat(msg.message, ";");

	if(count == 0)
		ret = sendMessage(client, &msg, 0);

	for(i = 0; i< count; i++)
	{
		if( (aux = wrappMedicine(p[i]->medicines, p[i]->destinationID, p[i]->medCount)) == NULL )
			return -1;
		itoa(p[i]->planeID, num);
		strcat(msg.message, num);
		strcat(msg.message, ";");
		strcat(msg.message, aux);
		free(aux);
	}
	
	if(count != 0)
		ret = sendMessage(client, &msg, 0);
	free(num);
	free(msg.message);

	return ret; 
}


int
rcvPlanes(int * companyID, int * count, plane *** p, clientADT client)
/*Format: companyID;count;destination1ID;medCount1;med1,c1;med2,c2;...;destination2ID;medCount2;med1,c1;...;*/
{
	message msg;
	int ret, i = 0, pos, j, medCount;
	char * aux = NULL;
	plane ** retPlane;

	if( (msg.message = calloc(MSG_SIZE, sizeof(char))) == NULL)
		return -1;
	msg.size = MSG_SIZE;
	
	if( (ret = rcvMessage(client, &msg, 0)) == -1 )
		return -1;

	if( (aux = calloc(10, sizeof(char))) == NULL)
		return -1;
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
	free(msg.message);

	return ret;
}


int
sendMap(int size, city ** cities, clientADT client)
{
	message msg;
	char * aux;
	int i;

	if( (msg.message = calloc(MSG_SIZE, sizeof(char))) == NULL)
		return -1;
	msg.size = MSG_SIZE;

	for(i = 0; i<size; i++)
	{
		if( (aux = wrappMedicine(cities[i]->medicines, cities[i]->ID, cities[i]->medCount)) == NULL)
			return -1;
		strcpy(msg.message, aux);
		sendMessage(client, &msg, 0);
		free(aux);
	}
	free(msg.message);

	return 0;
}


int
rcvMap(medicine **** meds, clientADT client, int size)
{
	message msg;
	int k = 0;
	medicine *** m = NULL;

	if( (msg.message = calloc(MSG_SIZE, sizeof(char))) == NULL )
		return -1;
	msg.size = MSG_SIZE;

	if( (m = malloc(sizeof(medicine *) * size)) == NULL)
		return -1;

	for(k = 0; k < size; k++)
	{
		if( rcvMessage(client, &msg, 0) == -1 )
			return -1;
		unwrappMedicine(&m[k], (char *)msg.message, NULL);
	}

	*meds = m;
	free(msg.message);

	return 0;
}


char *
wrappMedicine(medicine ** med, int ID, int medCount)
/*format: ID;medCount;med1,cant;med2,cant;...0*/
{
	int i;
	char * number = NULL;
	char * aux = NULL;

	if( (aux = calloc(10, sizeof(char))) == NULL)	/*set to serialize ID and medCount. initial size may be increased*/
		return NULL;
	if( (number = calloc(10, sizeof(char))) == NULL)
		return NULL;
	
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
unwrappMedicine(medicine *** meds, char * array, int * ID)
{
	int i = 0, k, pos = 0, size;
	char * aux;
	medicine ** m;
	
	if( (aux = calloc(MED_NAME_LENGT, sizeof(char))) == NULL)
		return -1;

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
	}
	m[pos] = NULL;

	*meds = m;
	free(aux);
	
	return i;
}
