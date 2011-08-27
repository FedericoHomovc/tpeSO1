/***
***
***		marshalling.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/

int sendPlanes(int companyID, int count, plane ** p, comuADT client);

int rcvPlanes(int * companyID, int * count, plane *** p, comuADT client);

int sendMap(int size, int ** map, city ** cities, comuADT client);

int rcvMap(int *** map, medicine **** meds, comuADT client, int * size);

int sendChecksign(comuADT client);

int rcvChecksign(comuADT client);
