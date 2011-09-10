/***
***
*** 		company.c
*** 			Jose Ignacio Galindo
*** 			Federico Homovc
*** 			Nicolas Loreti
*** 			     ITBA 2011
***
***/

/*** 		System includes 		***/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <math.h>

/*** 		Project Includes 		***/
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/api.h"
#include "./include/marshalling.h"

/*** 		Functions 		***/

/*
* function sigintServHandler
*
* Handler for SIGINT signal. Calls freeCompanyResources().
*/
static void sigintServHandler(int signo);

/*
* function freeCompanyResources
*
* Frees all the possible remaining memory alloc'd. 
*/
static void freeCompanyResources(void);

/*
* function threadFunc
*
* Function that performs all the planes operations like setting a new destination and
* updating cargo. It is synchronized so that only one plane thread can be running each
* time.
*
* @threadId: Plane ID (from 0 to 9).
*/
void * threadFunc(void * threadId);



int planesChecked, unloading, companyWorking = TRUE;
pthread_mutex_t mutexVar, planeMutex;
pthread_cond_t condVar, planeCond;
map * mapSt;
plane ** planes, ** receivedPlanes;
medicine *** med;
company * compa;
pthread_t * threads;
clientADT * clients, client;
serverADT server;
pid_t * pids;

int companyFunc(char * fileName, int companyID) {
	FILE * file = NULL;
	int i, j;

	struct sigaction signalAction;
	signalAction.sa_flags = 0;
	signalAction.sa_handler = sigintServHandler;
	sigfillset(&(signalAction.sa_mask));

	sigaction(SIGINT, &signalAction, NULL);

	if( (client = connectToServer(server)) == NULL )
	{
		fprintf(stderr, "Company %d couldn't connect to server.", companyID);
		return 1;
	}

	if( (clients[0] = getClient(server, getppid())) == NULL )
	{
		fprintf(stderr, "Map client not found\n");
		return 1;
	}

	sendChecksign(clients[0]);
	if (!openFile(&file, fileName)){
		if (createCompany(file, &compa)) {
			fprintf(stderr, "File Error\n");
			return 1;
		}
		fclose(file);
	} else {
		fprintf(stderr, "Impossible to open file\n");
		return 1;
	}
	compa->ID = companyID;
	for (i = 0; i < compa->planesCount; i++)
		if ((compa->companyPlanes[i]->destinationID = getCityID(compa->companyPlanes[i]->startCity, mapSt)) == -1) {
			fprintf(stderr, "Invalid start city for plane %d in company %d.\n", i, compa->ID);
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

	pthread_mutex_lock(&mutexVar);		/*create planes and wait for them to be initialized*/
	planesChecked = 0;
	for(i = 0; i < compa->planesCount; i++)
		pthread_create(&threads[i], NULL, threadFunc, (void*)i);
	while(planesChecked < compa->planesCount)
		pthread_cond_wait(&condVar, &mutexVar);
	pthread_mutex_unlock(&mutexVar);

	while (TRUE) {
		unloading = 0;
		rcvMap(&med, client, mapSt->citiesCount);

		pthread_mutex_lock(&planeMutex); /*wake up planes and wait until they check cargo*/
		pthread_mutex_lock(&mutexVar);
		planesChecked = 0;
		pthread_cond_broadcast(&planeCond);
		pthread_mutex_unlock(&planeMutex);
		while(planesChecked < compa->planesCount)
			pthread_cond_wait(&condVar, &mutexVar);
		pthread_mutex_unlock(&mutexVar);

		for (i = 0; i < mapSt->citiesCount; i++) {
			for (j = 0; med[i][j] != NULL; j++) {
				free(med[i][j]->name);
				free(med[i][j]);
			}
			free(med[i]);
		}
		free(med);

		if (!unloading)
			sendPlanes(compa->ID, unloading, NULL, clients[0]);

		else {
			sendPlanes(compa->ID, unloading, planes, clients[0]);
			rcvPlanes(NULL, &unloading, &receivedPlanes, client);
		}	

		pthread_mutex_lock(&planeMutex); /*wake up planes and wait until they update cargo*/
		pthread_mutex_lock(&mutexVar);
		planesChecked = 0;
		pthread_cond_broadcast(&planeCond);
		pthread_mutex_unlock(&planeMutex);
		while(planesChecked < compa->planesCount)
			pthread_cond_wait(&condVar, &mutexVar);
		pthread_mutex_unlock(&mutexVar);

		if(unloading)
		{
			for (i = 0; i < unloading; i++) {
				for (j = 0; j < receivedPlanes[i]->medCount; j++) {
					free(receivedPlanes[i]->medicines[j]->name);
					free(receivedPlanes[i]->medicines[j]);
				}
				free(receivedPlanes[i]->medicines);
				free(receivedPlanes[i]);
			}
			free(receivedPlanes);
		}

		rcvMap(&med, client, mapSt->citiesCount);

		pthread_mutex_lock(&planeMutex); /*wake up planes and wait until they set new destination*/
		pthread_mutex_lock(&mutexVar);
		planesChecked = 0;
		pthread_cond_broadcast(&planeCond);
		pthread_mutex_unlock(&planeMutex);
		while(planesChecked < compa->planesCount)
			pthread_cond_wait(&condVar, &mutexVar);
		pthread_mutex_unlock(&mutexVar);

		for (i = 0; i < mapSt->citiesCount; i++) {
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

	int ID, i, j, notUnloaded, rcvPlane, next, hasMedicine, dest;
	plane ** p;
	double max, acum;

	ID = (int)threadId;
	p = &(compa->companyPlanes[ID]);

	pthread_mutex_lock(&mutexVar);
	pthread_mutex_lock(&planeMutex);
	planesChecked++;
	pthread_cond_signal(&condVar);
	pthread_mutex_unlock(&mutexVar);

	while(companyWorking)
	{
		pthread_cond_wait(&planeCond, &planeMutex); /*wait first instruction*/
		pthread_mutex_unlock(&planeMutex);


		pthread_mutex_lock(&mutexVar);
		pthread_mutex_lock(&planeMutex);

		notUnloaded = hasMedicine = 1;
		rcvPlane = max = -1;
		acum = 0;
		planesChecked++;
		if((*p)->distance >= 0)
			(*p)->distance--;
		if ((*p)->distance == -1)
		{
			for (i = 0; med[(*p)->destinationID][i] != NULL && notUnloaded; i++)
				if (med[(*p)->destinationID][i]->quantity != 0)
					for (j = 0; j < (*p)->medCount && notUnloaded; j++)
						if ((*p)->medicines[j]->quantity > 0 && !strcmp((*p)->medicines[j]->name, med[(*p)->destinationID][i]->name))
						{
							planes[unloading++] = compa->companyPlanes[ID];
							notUnloaded = 0;
						}
		}

		pthread_cond_signal(&condVar); /*cargo checked, wake up company*/
		pthread_mutex_unlock(&mutexVar);

		pthread_cond_wait(&planeCond, &planeMutex); /*wait*/
		pthread_mutex_unlock(&planeMutex);

		
		pthread_mutex_lock(&mutexVar);
		pthread_mutex_lock(&planeMutex);
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

		pthread_cond_signal(&condVar); /*cargo updated, wake up company*/
		pthread_mutex_unlock(&mutexVar);

		pthread_cond_wait(&planeCond, &planeMutex); /*wait*/
		pthread_mutex_unlock(&planeMutex);
		

		pthread_mutex_lock(&mutexVar);
		pthread_mutex_lock(&planeMutex);
		planesChecked++;

		if((*p)->distance == -1)
		{
			for( next = 0; next < mapSt->citiesCount; next++)
			{
				notUnloaded = 1;
				for(j = 0; med[next][j] != NULL; j++)
					for(i = 0; i < (*p)->medCount; i++)
						if( med[next][j]->quantity > 0 && (*p)->medicines[i]->quantity > 0 && 
							!strcmp((*p)->medicines[i]->name, med[next][j]->name))
						{
							acum += fabs(med[next][j]->quantity - (*p)->medicines[i]->quantity);
							hasMedicine = notUnloaded = 0;
						}
				if( mapSt->graph[(*p)->destinationID][next] != 0)
					acum /= mapSt->graph[(*p)->destinationID][next];
				if(acum > max && !notUnloaded)
				{
					max = acum;
					dest = next;
				}
			}
			if(hasMedicine)
				(*p)->distance--;
			else{
				(*p)->distance = mapSt->graph[(*p)->destinationID][dest];
				(*p)->destinationID = dest;
			}	
		}

		pthread_cond_signal(&condVar); /*new destination set, wake up company*/
		pthread_mutex_unlock(&mutexVar);
	}

	pthread_exit(NULL);
	return NULL;
}

static void sigintServHandler(int signo) {

	companyWorking = FALSE;
	disconnectFromServer(client, server);
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
		free(compa->companyPlanes[i]->medicines);
		free(compa->companyPlanes[i]->startCity);
		free(compa->companyPlanes[i]);
	}
	free(compa->companyPlanes);
	free(compa);

	for(i = 0; i < mapSt->citiesCount; i++)
		free(mapSt->graph[i]);	
	free(mapSt->graph);
	for(i = 0; i < mapSt->citiesCount; i++)
	{
		for(j = 0; j < mapSt->cities[i]->medCount; j++)
		{
			free( mapSt->cities[i]->medicines[j]->name);
			free(mapSt->cities[i]->medicines[j]);
		}
		free(mapSt->cities[i]->medicines);
		free(mapSt->cities[i]->name);
		free(mapSt->cities[i]);
	}
	free(mapSt->cities);
	free(mapSt);

	free(threads);
	free(planes);
	free(clients);
	free(pids);
	free(server);

	return;
}
