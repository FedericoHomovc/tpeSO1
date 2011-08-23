/***
***
***		marshalling.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

int sendPackage(int city, medicine ** med, comuADT client, int companyID, int planeID, int medCount);

int rcvPackage(int * city, medicine ** med, comuADT client, int * companyID, int * planeID );

int sendMap(int size, int ** map, city ** cities, comuADT client);

int rcvMap(int *** map, medicine **** meds, comuADT client, int * size);
