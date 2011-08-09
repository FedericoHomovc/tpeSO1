/***
***
***		backEnd.h
***				Jose Ignacio Galindo
***				Federico Homovc
***				Nicolas Loreti
***			 	     ITBA 2011
***
***/


int allocMapData(mapData ** mapFile);

int openFile(mapData * game, char * fName);

/*allocs memory for the map and the cities*/
int allocMapSt(map ** mapSt, int argc);

int createCities(mapData * mapFile, map * mapSt);

int createCompany(mapData * mapFile, company ** newCompany);

int initializeGraph(mapData * mapFile, map * mapSt);

int getCityID( char * cityName, map * mapSt);

int createPlane(mapData * mapFile, plane ** newPlane);

