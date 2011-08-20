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
rcvMap(int ** map, comuADT client, int * size)
{
	message msg;
	int ret;

	if( (msg.message = malloc( sizeof(char *) )) == NULL )
		return -1;
	if( (ret = rcvMsg(client, &msg, 0)) == -1 )
		return -1;

	/*---------TESTING----------*/
	printf("map rcv: %s\n", (char *)msg.message);
	printf("rcv size: %ld\n", msg.size);
	/*---------TESTING----------*/

	if( (map = unwrappMap((char *) msg.message, size)) == NULL)
		return -1;

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
	char * aux = NULL;
	int ** map = NULL;
	int i = 0, k, size, number, pos = 0;
	
	aux = malloc(10);
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
		/*---------TESTING----------*/
		printf("num: %d\n", map[pos/size][pos%size]);
		/*---------TESTING----------*/
		pos++; i++;
	}

	free(aux);
	*retSize = size;

	return map;
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


