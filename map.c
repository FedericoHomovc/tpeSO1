/***
 ***
 ***		frontEnd.c
 ***			Jose Ignacio Galindo
 ***			Federico Homovc
 ***			Nicolas Loreti
 ***			     ITBA 2011
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

/***		Project Includes		***/
#include "./include/api.h"
#include "./include/structs.h"
#include "./include/backEnd.h"
#include "./include/marshalling.h"

/***		Functions		***/

/*
* function companyFunc
*
* Function to be executed once the company process is forked. It performs all
* the companies operations like setting planes destinations and unloading 
* their cargo.
*
* @fileName: Name of the company file name.
* @companyID: Company ID (from 0 to 9).
*/
int companyFunc(char * fileName, int companyID);

/*
* function ioFunc
*
* Function to be executed once the Imput/Output process is forked. Receives the 
* medicines from each city and the planes that unloaded cargo in each turn and 
* prints it on screen.
*/
int ioFunc(void);

/*
* function unloadPlane
*
* Checks if the plane can unload cargo in its destination city and, if so, updates both
* the city and plane's quantity of each correspondig medicine.
*
* @p: Plane arrived to its destination. 
* @mapSt: Map struct with all the current map information.
*/
static int unloadPlane(plane ** p, map ** mapSt);

/*
* function freeResources
*
* Frees all the possible remaining memory alloc'd. 
*/
static int freeResources(void);

/*
* function needMedicines
*
* Checks if any city in the map still needs any medicine.
*
* @mapSt: Map struct with all the current map information.
*/
static int needMedicines(map * mapSt);

/*
* function sigintServHandler
*
* Handler for SIGINT signal. Sends SIGINT to all child processes and calls freeResources().
*/
static void sigintServHandler(int signo);



map * mapSt;
pid_t * pids;
serverADT server;
clientADT * clients;

int main(int argc, char * argv[]) {

	int k, companyID, count, i, j, c, unloaded;
	FILE * file = NULL;
	plane ** p;
	struct timeval t1, t2;
	double elapsedTime;

	/* start timer */
	gettimeofday(&t1, NULL);

	struct sigaction signalAction;
	signalAction.sa_flags = 0;
	signalAction.sa_handler = sigintServHandler;
	sigfillset(&(signalAction.sa_mask));
	sigaction(SIGINT, &signalAction, NULL);

	if (argc <= 2) {
		printf("Invalid arguments\n");
		return 1;
	}

	if (!openFile(&file, argv[1])) {
		if ((mapSt = malloc(sizeof(map))) == NULL )
			return 1;
		if (createCities(file, mapSt)) {
			fprintf(stderr, "Map File Error\n");
			return 1;
		}
		fclose(file);
	} else {
		fprintf(stderr, "Impossible to open file\n");
		return 1;
	}
	mapSt->companiesCount = argc - 2;

	if ((server = startServer()) == NULL )
		return 1;

	if ((pids = malloc(sizeof(pid_t) * (argc))) == NULL )
		return 1;
	if ((clients = malloc(sizeof(clientADT *) * (argc))) == NULL )
		return 1;

	pids[0] = getpid();
	if ((clients[0] = connectToServer(server)) == NULL) /*map client*/
	{
		return 1;
	}

	switch (pids[1] = fork()) {
	case -1:
		perror("Error creating IO");
		exit(1);
	case 0:
		if (ioFunc())
			fprintf(stderr, "Error during IO execution\n");
		_exit(0);
	default:
		k = 2;
		while (k < argc) {
			switch (pids[k] = fork()) {
			case -1:
				perror("Error creating company");
			case 0:
				if (companyFunc(argv[k], k - 2))
					fprintf(stderr, "Error during company %d execution\n", k - 2);
				_exit(0);
			}
			k++;
		}
	}

	k = 0;
	while (k < argc - 1) /*wait for all processes to connect to server*/
	{
		if (rcvChecksign(clients[0]) == -1) {
			printf("Error connecting processes.\n");
			return 1;
		}
		k++;
	}

	if ((clients[1] = getClient(server, pids[1])) == NULL) /*ioClient*/
	{
		printf("Error connecting processes. PID %d not connected.\n", pids[1]);
		return 1;
	}
	for (k = 2; k < argc; k++)
		if ((clients[k] = getClient(server, pids[k])) == NULL)
		{
			printf("Error connecting processes. PID %d not connected.\n",
					pids[k]);
			return 1;
		}

	while (needMedicines(mapSt)) {
		sendMap(mapSt->citiesCount, mapSt->cities, clients[1]);
		if (rcvChecksign(clients[0]) == -1) {
			printf("Error during IPC @ map.c\n");
			return 1;
		}

		/*do{
		 c = getchar();
		 }while(c != '\n');*/

		for (k = 2; k < argc; k++)
			sendMap(mapSt->citiesCount, mapSt->cities, clients[k]);

		k = 0;
		while (k < argc - 2) {
			rcvPlanes(&companyID, &count, &p, clients[0]);
			unloaded = 0;
			if (count > 0) {
				for (i = 0; i < count; i++)
					unloaded += unloadPlane(&p[i], &mapSt);

				sendPlanes(companyID, count, p, clients[companyID + 2]);
			}
			if (unloaded) {
				sendPlanes(companyID, count, p, clients[1]);
				for (i = 0; i < count; i++) {
					for (j = 0; j < p[i]->medCount; j++) {
						free(p[i]->medicines[j]->name);
						free(p[i]->medicines[j]);
					}
					free(p[i]->medicines);
					free(p[i]);
				}
				free(p);
			} else
				sendPlanes(companyID, 0, NULL, clients[1]);
			k++;
		}

		for (k = 2; k < argc; k++)
			sendMap(mapSt->citiesCount, mapSt->cities, clients[k]);

		if (rcvChecksign(clients[0]) == -1) {
			printf("Error during IPC @ map.c\n");
			return 1;
		}
	}

	sendMap(mapSt->citiesCount, mapSt->cities, clients[1]);
	rcvChecksign(clients[0]);

	for (k = 1; k < argc; k++) {
		kill(pids[k], SIGINT);
		disconnectFromServer(clients[k], server);
	}
	disconnectFromServer(clients[0], server);
	endServer(server);

	freeResources();
	printf("\nSimulation ended\n");
	/* stop timer */
	gettimeofday(&t2, NULL);

	/* compute and print the elapsed time in millisec */
	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0; /* sec to ms */
	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0; /* us to ms */
	printf("Elapsed time: %f miliseconds\n", elapsedTime);
	return 0;
}

static int unloadPlane(plane ** p, map ** mapSt) {
	int i, j, ret = 0;
	city * cty;

	cty = (*mapSt)->cities[(*p)->destinationID];

	for (i = 0; i < (*mapSt)->cities[(*p)->destinationID]->medCount; i++) {
		for (j = 0; (*p)->medicines[j] != NULL; j++) {
			if (cty->medicines[i]->quantity > 0
					&& (*p)->medicines[j]->quantity > 0
					&& !strcmp((*p)->medicines[j]->name,
							cty->medicines[i]->name)) {
				if ((*p)->medicines[j]->quantity
						>= cty->medicines[i]->quantity) {
					(*p)->medicines[j]->quantity -= cty->medicines[i]->quantity;
					cty->medicines[i]->quantity = 0;
				} else {
					cty->medicines[i]->quantity -= (*p)->medicines[j]->quantity;
					(*p)->medicines[j]->quantity = 0;
				}
				ret = 1;
			}
		}
	}

	return ret;
}

static int freeResources(void) {
	int i, j;
	for (i = 0; i < mapSt->citiesCount; i++)
		free(mapSt->graph[i]);
	free(mapSt->graph);
	for (i = 0; i < mapSt->citiesCount; i++) {
		for (j = 0; j < mapSt->cities[i]->medCount; j++) {
			free(mapSt->cities[i]->medicines[j]->name);
			free(mapSt->cities[i]->medicines[j]);
		}
		free(mapSt->cities[i]->medicines);
		free(mapSt->cities[i]->name);
		free(mapSt->cities[i]);
	}
	free(mapSt->cities);
	free(mapSt);
	free(clients);
	free(pids);

	return 0;
}

static void sigintServHandler(int signo) {
	int k;

	for (k = 1; k < mapSt->companiesCount + 2; k++) {
		kill(pids[k], SIGINT);
		disconnectFromServer(clients[k], server);
	}
	disconnectFromServer(clients[0], server);
	endServer(server);
	freeResources();

	exit(0);
}

static int needMedicines(map * mapSt) {
	int i, j;
	for (i = 0; i < mapSt->citiesCount; i++)
		for (j = 0; j < mapSt->cities[i]->medCount; j++)
			if (mapSt->cities[i]->medicines[j]->quantity > 0)
				return 1;

	return 0;
}

