/***
 ***
 ***		frontEnd.c
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

/***		Project Includes		***/
#include "./include/api.h"
#include "./include/structs.h"
#include "./include/backEnd.h"

int companyFunc(processData * pdata, char * fileName, int companyID);
int rcvPackage(int * city, medicine ** med, comuADT client, int * companyID,
		int * planeID);

int main(int argc, char * argv[]) {

	processData * pdata;
	int k, notValid;
	pid_t * pids;
	servADT server;
	comuADT * clients;
	mapData * mapFile;
	map * mapSt;

	server = startServer();
	pdata = malloc(sizeof(processData));
	pdata->server = server;
	pdata->pid = getpid();
	pdata->name = malloc(sizeof(char *) * 20); /* tamaño del string */

	pids = malloc(sizeof(pid_t) * (argc)); /*poner en el back*/
	pids[0] = getpid();
	clients = malloc(sizeof(comuADT *) * (argc)); /*poner en el back*/

	if (argc <= 2) {
		printf("Invalid arguments\n");
		return 1;
	}

	printf("soy el mapa\n");
	strcpy(pdata->name, "map");

	if (allocMapData(&mapFile)) {
		return 1;
	}
	if (!openFile(mapFile, argv[1])) {
		allocMapSt(&mapSt, argc);
		notValid = createCities(mapFile, mapSt);
		if (notValid) {
			printf("File Error\n");
			return notValid;
		}
	} else {
		printf("Impossible to open file\n");
		return 1;
	}

	switch (pids[1] = fork()) {
	case -1:
		perror("creating IO");
		exit(1);
	case 0:
		printf("empieza IO\n");
		ioFunc(pdata);
		/*execl("io", "io", (char *) 0);*/
		_exit(0);
	default:
		k = 2;
		while (k < argc) {
			switch (pids[k] = fork()) {
			case -1:
				perror("creating company");
			case 0:
				printf("empieza la company %d\n", k - 2);
				if (companyFunc(pdata, argv[k], k - 2))
					printf("error opening company %d file\n", k);
				/*execl("company", "company", argv[k+2], (char *) 0);*/
				_exit(0);
			}
			k++;
		}
	}
	sleep(2);

	clients[0] = connectToServer(server); /*map client*/
	clients[1] = getClient(server, pids[1]);/*ioClient*/
	for (k = 2; k < argc; k++) {
		clients[k] = getClient(server, pids[k]);
	}

	/*---------TESTING----------*/
	int city;
	int companyID;
	int planeID;
	medicine * med;

	sleep(2);
	printf("%d\n", rcvPackage(&city, &med, clients[2], &companyID, &planeID));

	/*printf("%s -- %d -- %d -- %d\n", med[0].name, med[0].quantity, city, companyID);*/
	/*---------TESTING----------*/

	/*	while (waitpid(pids[0], &status, WNOHANG) == 0) {
	 printf("waiting for map to end...\n");
	 sleep(1);
	 }
	 if (WIFEXITED(status)) {
	 printf("termino el mapa\n");
	 disconnectFromServer(clients[0], server);
	 kill(pids[0], SIGTERM);
	 kill(pids[1], SIGTERM);
	 endServer(server);

	 } */
	printf("termino el mapa\n");
	/*disconnectFromServer(clients[0], server);*/
	kill(pids[1], SIGTERM);
	kill(pids[2], SIGTERM);
	endServer(server);

	/*wait(&rv);
	 printf("termina el mapa\n");*/

	/*	startSimulation();*/
	/*	freeResources();*/

	return 0;
}
