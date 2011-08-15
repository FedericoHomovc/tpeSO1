/***		System includes		***/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/api.h"



int 
sendParseFile(char * fileName, comuADT client)
{
	messaje msj = {strlen(fileName), filename};
	

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


