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
#include <sys/types.h>
#include <sys/wait.h>
#include <error.h>
#include <pthread.h>

/***		Project Includes		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/api.h"
#include "./include/marshalling.h"

/***		Functions		***/
static void sigintServHandler(int signo);
void * threadFunc(void * threadId);
void freeCompanyResources(void);



int planesChecked, unloading, size;
pthread_mutex_t mutexVar, planeMutex;
pthread_cond_t condVar, planeCond;
map * mapSt;
comuADT mapClient;
plane ** planes, ** receivedPlanes;
medicine *** med;
company * compa;
pthread_t * threads;

int companyFunc(processData * pdata, char * fileName, int companyID) {
	FILE * file = NULL;
	comuADT client;	
	int i, j;

	struct sigaction signalAction;
	signalAction.sa_flags = 0;
	signalAction.sa_handler = sigintServHandler;
	sigfillset(&(signalAction.sa_mask));

	sigaction(SIGINT, &signalAction, NULL);

	client = connectToServer(pdata->server);
	sendChecksign(mapClient);

	if (!openFile(&file, fileName)){
		if (createCompany(file, &compa)) {
			printf("File Error\n");
			return 1;
		}
		/*fclose(file);*/
	} else {
		printf("Impossible to open file\n");
		return 1;
	}
	compa->ID = companyID;
	for (i = 0; i < compa->planesCount; i++)
		if ((compa->companyPlanes[i]->destinationID = getCityID(
				compa->companyPlanes[i]->startCity, mapSt)) == -1) {
			printf("Invalid start city for plane %d in company %d.\n", i,
					compa->ID);
			return 1;
		}

	if( (planes = malloc(sizeof(plane *) * compa->planesCount)) == NULL)
		return 1;
	if( (threads = malloc(sizeof(pthread_t *) * compa->planesCount)) == NULL)
		return 1;

	pthread_mutex_init(&mutexVar, NULL);
	pthread_cond_init(&condVar, NULL);
	pthread_mutex_init(&planeMutex, NULL);
	pthread_cond_init(&planeCond, NULL);

	for(i = 0; i < compa->planesCount; i++)		/*falta esperar a que se creen todos*/
		pthread_create(&threads[i], NULL, threadFunc, (void*)i);

	while (TRUE) {
		planesChecked = 0;
		unloading = 0;
		rcvMap(&med, client, &size);

		pthread_mutex_lock(&mutexVar);
		pthread_cond_broadcast(&planeCond);
		while(planesChecked < compa->planesCount)
			pthread_cond_wait(&condVar, &mutexVar);

		for (i = 0; i < size; i++) {
			for (j = 0; med[i][j] != NULL; j++) {
				free(med[i][j]->name);
				free(med[i][j]);
			}
			free(med[i]);
		}
		free(med);
		
		if (!unloading)
			sendPlanes(compa->ID, unloading, NULL, mapClient);

		else {
			sendPlanes(compa->ID, unloading, planes, mapClient);
			rcvPlanes(NULL, &unloading, &receivedPlanes, client);
		}

		planesChecked = 0;

		pthread_mutex_lock(&planeMutex);
		pthread_cond_broadcast(&planeCond);
		pthread_mutex_unlock(&planeMutex);
		while(planesChecked < compa->planesCount)
			pthread_cond_wait(&condVar, &mutexVar);
		pthread_mutex_unlock(&mutexVar);

		if(unloading)
			for (i = 0; i < unloading; i++) {
				for (j = 0; j < receivedPlanes[i]->medCount; j++) {
					free(receivedPlanes[i]->medicines[j]->name);
					free(receivedPlanes[i]->medicines[j]);
				}
				free(receivedPlanes[i]);
			}

		rcvMap(&med, client, &size);

		planesChecked = 0;

		pthread_mutex_lock(&planeMutex);
		pthread_cond_broadcast(&planeCond);
		pthread_mutex_unlock(&planeMutex);
		while(planesChecked < compa->planesCount)
			pthread_cond_wait(&condVar, &mutexVar);
		pthread_mutex_unlock(&mutexVar);

		for (i = 0; i < size; i++) {
			for (j = 0; med[i][j] != NULL; j++) {
				free(med[i][j]->name);
				free(med[i][j]);
			}
			free(med[i]);
		}
		free(med);
	}

	return 1;
}

void * threadFunc(void * threadId){

	int ID, i, j, notUnloaded, rcvPlane, next;
	plane ** p;

	ID = (int)threadId;
	p = &(compa->companyPlanes[ID]);
	
	while(TRUE)
	{
		pthread_mutex_lock(&planeMutex);
		pthread_cond_wait(&planeCond,&planeMutex);
		
		notUnloaded = next = 1;
		rcvPlane = -1;
		planesChecked++;
		(*p)->distance--;
		if ((*p)->distance < 0)
		{
			for (i = 0; med[(*p)->destinationID][i] != NULL && notUnloaded; i++)
				if (med[(*p)->destinationID][i]->quantity != 0)
					for (j = 0; j < (*p)->medCount && notUnloaded; j++)
						if (!strcmp((*p)->medicines[j]->name, med[(*p)->destinationID][i]->name))
						{
							planes[unloading++] = compa->companyPlanes[ID];
							notUnloaded = 0;
						}
		}
	
		pthread_mutex_lock(&mutexVar);
		pthread_cond_signal(&condVar);		/*le digo a company que siga*/
		pthread_mutex_unlock(&mutexVar);
		
		pthread_cond_wait(&planeCond,&planeMutex);	/*me quedo esperando*/
		planesChecked++;
		
		if(!notUnloaded)
		{
			for( i = 0; i < unloading; i++ )
				if((*p)->planeID == receivedPlanes[i]->planeID)
					rcvPlane = i;
			if( i == -1)
				printf("Error fatal\n");
			for (j = 0; j < (*p)->medCount; j++)
				(*p)->medicines[j]->quantity = receivedPlanes[rcvPlane]->medicines[j]->quantity;
		}

		pthread_mutex_lock(&mutexVar);
		pthread_cond_signal(&condVar);		/*le digo a company que siga*/
		pthread_mutex_unlock(&mutexVar);

		pthread_cond_wait(&planeCond,&planeMutex);	/*me quedo esperando*/
		planesChecked++;

		if((*p)->distance == -1)
		{
			while ( mapSt->graph[ (*p)->destinationID ][ ((*p)->destinationID + next) % size ] == 0)
				next++;
			(*p)->distance = mapSt->graph[(*p)->destinationID][((*p)->destinationID + next) % size];
			(*p)->destinationID = ((*p)->destinationID + next) % size;
		}

		pthread_mutex_lock(&mutexVar);
		pthread_cond_signal(&condVar);		/*le digo a company que siga*/
		pthread_mutex_unlock(&mutexVar);

		pthread_mutex_unlock(&planeMutex);
	}

	pthread_exit(NULL);
	return NULL;
}

static void sigintServHandler(int signo) {
	int i;
	for(i = 0; i < compa->planesCount; i++)
		pthread_cancel(threads[i]);		/*hay que cancelar los threads pero creo que no anda esto*/
	freeCompanyResources();
	
	exit(0);
}

void
freeCompanyResources(void)
{
	int i, j;
	for(i = 0; i < compa->planesCount; i++)
	{
		for (j = 0; j < compa->companyPlanes[i]->medCount; j++) {
			free(compa->companyPlanes[i]->medicines[j]->name);
			free(compa->companyPlanes[i]->medicines[j]);
		}
		free(compa->companyPlanes[i]);
	}
	free(compa);

	for(i = 0; i < mapSt->citiesCount; i++)	/*si es la misma direccion de memoria que map.c para mapSt no hace falta liberar*/
		free(mapSt->graph[i]);	
	free(mapSt->graph);
	for(i = 0; i < mapSt->citiesCount; i++)
	{
		for(j = 0; j < mapSt->cities[i]->medCount; j++)
		{
			free( mapSt->cities[i]->medicines[j]->name);
			free(mapSt->cities[i]->medicines[j]);
		}
		free(mapSt->cities[i]);
	}
	free(mapSt);

	free(planes);
	free(receivedPlanes);

	return;
}
