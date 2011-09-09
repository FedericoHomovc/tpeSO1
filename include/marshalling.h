/***
***
***		marshalling.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

/***		Functions		***/
int sendPlanes(int companyID, int count, plane ** p, clientADT client);

int rcvPlanes(int * companyID, int * count, plane *** p, clientADT client);

int sendMap(int size, city ** cities, clientADT client);

int rcvMap(medicine **** meds, clientADT client, int size);

int sendChecksign(clientADT client);

int rcvChecksign(clientADT client);
