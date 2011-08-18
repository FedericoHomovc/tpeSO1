/***		System includes		***/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>


#include "../include/api.h"
#include "../include/structs.h"


int wrappMedicine(medicine ** med, char ** array, int medCount);
int unwrappMedicine(medicine ** med, char * array);
void itoa(int n, char *string);


int
sendPackage(int city, medicine ** med, comuADT client, int companyID, int planeID, int medCount)
{
	package pack;
	message msg;

	pack.city = city;
	pack.companyID = companyID;
	pack.planeID = planeID;

	if( wrappMedicine(med, &pack.med, medCount) == -1)
		return -1;

	/*---------TESTING----------*/
	printf("pack med: %s\n", pack.med);
	/*---------TESTING----------*/
	
	msg.message = (package*)&pack;
	msg.size = sizeof(package);

	return sendMsg(client, &msg, 0);
}


int
rcvPackage(int * city, medicine ** med, comuADT client, int * companyID, int * planeID )
{
	message msg;
	int ret;

	msg.message = malloc( sizeof(package) );
	msg.size = sizeof(package);
	if( (ret = rcvMsg(client, &msg, 0)) == -1 )
	{
		return 1;
	}

	/*unwrappMedicine(med, ((package *)msg.message)->med);*/
	*city = ((package *)msg.message)->city;
	*companyID = ((package *)msg.message)->companyID;
	*planeID = ((package *)msg.message)->planeID;

	return ret;

}


int
wrappMedicine(medicine ** med, char ** array, int medCount)  /*formato: med1,cant;med2,cant;med3,cant...0 */
{
	int i;
	char * number = NULL;
	char * aux = NULL;

	aux = malloc(1);
	number = malloc(10);

	for( i = 0; i < medCount; i++)
	{
		if( ( aux = realloc(aux, strlen(aux) + strlen(med[i]->name) + sizeof(int) + 2)) == NULL )
			return -1;
		strcat(aux, med[i]->name);
		strcat(aux, ",");
		itoa(med[i]->quantity, number);
		if( aux == NULL )
			return -1;
		strcat(aux, number);
		strcat(aux, ";");
	}
	*array = aux;
	free(number);
	
	return i;	
}

int
unwrappMedicine(medicine ** med, char * array)
{
	return -1;
}

/*void main(void){
	
	comuADT comuArray[2] = {0};
	pid_t pid;
	pid_t pidArray[2] = {0};

	int i,j,z;

	pidArray[0] = getpid();
	servADT server = startServer();


	for( i = 1; i < 2 ; i++){
		
		pid = fork();
		if ( pid > 0 ){
			break;		
		}else{
			printf("Soy el hijo %d\n", i);
			printf("Serverasdasdadsad: %s", (char *)server);
			pidArray[i] = getpid();
			sleep(4);
		}
	}
	
	printf("Me estoy conectando al server y soy el hijo %d\n",i);
	connectToServer(server);
	printf("Ya me conecte y soy el hijo %d\n",i);
	
	for ( j = 0; j<2; j++){
		printf("Update de los procesos que estan corriendo y su numero de PID:\nEl hijo %d tiene un pid:%d\n", j+1, pidArray[j]);
	}
	
	//for( z = 0; z < 2; z++)
	//{	
		//pid_t pid;
		pid = fork();
		if (pid == 0)
		{ 
			printf("Hijo presente\n"); 
			char * string = "esteelmensajemensaje";
			comuADT cliente1 = connectToServer(server);
			message mensaje = {20,string};
			sendMsg(cliente1,&mensaje,0);
			printf("Hijo: Mande el mensaje\n");
		}else{
			sleep(5);
			printf("Aca el padre\n");
			char * resp = (char *)malloc(sizeof(20));
			printf("El tamanio de resp es:%d\n",sizeof(resp));
			message respuesta = {20,resp};
			comuADT nuevoCliente = connectToServer(server);
			nuevoCliente = getClient(server,pid);	
			int rec = rcvMsg(nuevoCliente,&respuesta,0);
			printf("La cantidad de caracteres que recibio:%d\n",rec);
			printf("%s\n",(char*)respuesta.message);
		}
	//}

	printf("termine\n");

}*/


