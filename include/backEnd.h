/***
***
***		backEnd.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/


/***		Functions		***/
int openFile(FILE ** file, char * fName);

int allocMapSt(map ** mapSt, int argc);

int createCities(FILE * mapFile, map * mapSt);

int createCompany(FILE * mapFile, company ** newCompany);

int initializeGraph(FILE * mapFile, map * mapSt);

int getCityID( char * cityName, map * mapSt);

int createPlane(FILE * mapFile, plane ** newPlane);
